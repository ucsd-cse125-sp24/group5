////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics.hpp>

#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

#include <algorithm>
#include <array>
#include <deque>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>


namespace
{
// Width and height of the application window
const unsigned int windowWidth  = 800;
const unsigned int windowHeight = 600;

// Resolution of the generated terrain
const unsigned int resolutionX = 800;
const unsigned int resolutionY = 600;

// Thread pool parameters
const unsigned int threadCount = 4;
const unsigned int blockCount  = 32;

struct WorkItem
{
    sf::Vertex*  targetBuffer{};
    unsigned int index{};
};

std::deque<WorkItem>     workQueue;
std::vector<std::thread> threads;
int                      pendingWorkCount    = 0;
bool                     workPending         = true;
bool                     bufferUploadPending = false;
std::recursive_mutex     workQueueMutex;

struct Setting
{
    const char* name{};
    float*      value{};
};

// Terrain noise parameters
const int perlinOctaves = 3;

float perlinFrequency     = 7.0f;
float perlinFrequencyBase = 4.0f;

// Terrain generation parameters
float heightBase          = 0.0f;
float edgeFactor          = 0.9f;
float edgeDropoffExponent = 1.5f;

float snowcapHeight = 0.6f;

// Terrain lighting parameters
float heightFactor  = windowHeight / 2.0f;
float heightFlatten = 3.0f;
float lightFactor   = 0.7f;
} // namespace


// Forward declarations of the functions we define further down
void threadFunction();
void generateTerrain(sf::Vertex* vertexBuffer);


////////////////////////////////////////////////////////////
/// Entry point of application
///
/// \return Application exit code
///
////////////////////////////////////////////////////////////
int main()
{
    // Create the window of the application
    sf::RenderWindow window(sf::VideoMode({windowWidth, windowHeight}), "SFML Island", sf::Style::Titlebar | sf::Style::Close);
    window.setVerticalSyncEnabled(true);

    sf::Font font;
    if (!font.loadFromFile("resources/tuffy.ttf"))
        return EXIT_FAILURE;

    // Create all of our graphics resources
    sf::Text               hudText(font);
    sf::Text               statusText(font);
    sf::Shader             terrainShader;
    const sf::RenderStates terrainStates(&terrainShader);
    sf::VertexBuffer       terrain(sf::PrimitiveType::Triangles, sf::VertexBuffer::Usage::Static);

    // Set up our text drawables
    statusText.setCharacterSize(28);
    statusText.setFillColor(sf::Color::White);
    statusText.setOutlineColor(sf::Color::Black);
    statusText.setOutlineThickness(2.0f);

    hudText.setCharacterSize(14);
    hudText.setFillColor(sf::Color::White);
    hudText.setOutlineColor(sf::Color::Black);
    hudText.setOutlineThickness(2.0f);
    hudText.setPosition({5.0f, 5.0f});

    // Staging buffer for our terrain data that we will upload to our VertexBuffer
    std::vector<sf::Vertex> terrainStagingBuffer;

    // Check whether the prerequisites are supported
    bool prerequisitesSupported = sf::VertexBuffer::isAvailable() && sf::Shader::isAvailable();

    // Set up our graphics resources and set the status text accordingly
    if (!prerequisitesSupported)
    {
        statusText.setString("Shaders and/or Vertex Buffers Unsupported");
    }
    else if (!terrainShader.loadFromFile("resources/terrain.vert", "resources/terrain.frag"))
    {
        prerequisitesSupported = false;

        statusText.setString("Failed to load shader program");
    }
    else
    {
        // Start up our thread pool
        for (unsigned int i = 0; i < threadCount; ++i)
        {
            threads.emplace_back(threadFunction);
        }

        // Create our VertexBuffer with enough space to hold all the terrain geometry
        if (!terrain.create(resolutionX * resolutionY * 6))
        {
            std::cerr << "Failed to create vertex buffer" << std::endl;
            return EXIT_FAILURE;
        }

        // Resize the staging buffer to be able to hold all the terrain geometry
        terrainStagingBuffer.resize(resolutionX * resolutionY * 6);

        // Generate the initial terrain
        generateTerrain(terrainStagingBuffer.data());

        statusText.setString("Generating Terrain...");
    }

    // Center the status text
    statusText.setPosition({(windowWidth - statusText.getLocalBounds().width) / 2.f,
                            (windowHeight - statusText.getLocalBounds().height) / 2.f});

    // Set up an array of pointers to our settings for arrow navigation
    constexpr std::array<Setting, 9> settings = {
        {{"perlinFrequency", &perlinFrequency},
         {"perlinFrequencyBase", &perlinFrequencyBase},
         {"heightBase", &heightBase},
         {"edgeFactor", &edgeFactor},
         {"edgeDropoffExponent", &edgeDropoffExponent},
         {"snowcapHeight", &snowcapHeight},
         {"heightFactor", &heightFactor},
         {"heightFlatten", &heightFlatten},
         {"lightFactor", &lightFactor}}};

    std::size_t currentSetting = 0;

    std::ostringstream osstr;
    sf::Clock          clock;

    while (window.isOpen())
    {
        // Handle events
        while (const auto event = window.pollEvent())
        {
            // Window closed or escape key pressed: exit
            if (event.is<sf::Event::Closed>() || (event.is<sf::Event::KeyPressed>() &&
                                                  event.getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape))
            {
                window.close();
                break;
            }

            // Arrow key pressed:
            if (prerequisitesSupported && event.is<sf::Event::KeyPressed>())
            {
                switch (event.getIf<sf::Event::KeyPressed>()->code)
                {
                    case sf::Keyboard::Key::Enter:
                        generateTerrain(terrainStagingBuffer.data());
                        break;
                    case sf::Keyboard::Key::Down:
                        currentSetting = (currentSetting + 1) % settings.size();
                        break;
                    case sf::Keyboard::Key::Up:
                        currentSetting = (currentSetting + settings.size() - 1) % settings.size();
                        break;
                    case sf::Keyboard::Key::Left:
                        *(settings[currentSetting].value) -= 0.1f;
                        break;
                    case sf::Keyboard::Key::Right:
                        *(settings[currentSetting].value) += 0.1f;
                        break;
                    default:
                        break;
                }
            }
        }

        // Clear, draw graphics objects and display
        window.clear();

        window.draw(statusText);

        if (prerequisitesSupported)
        {
            {
                const std::lock_guard lock(workQueueMutex);

                // Don't bother updating/drawing the VertexBuffer while terrain is being regenerated
                if (!pendingWorkCount)
                {
                    // If there is new data pending to be uploaded to the VertexBuffer, do it now
                    if (bufferUploadPending)
                    {
                        if (!terrain.update(terrainStagingBuffer.data()))
                        {
                            std::cerr << "Failed to update vertex buffer" << std::endl;
                            return EXIT_FAILURE;
                        }

                        bufferUploadPending = false;
                    }

                    terrainShader.setUniform("lightFactor", lightFactor);
                    window.draw(terrain, terrainStates);
                }
            }

            // Update and draw the HUD text
            osstr.str("");
            osstr << "Frame:  " << clock.restart().asMilliseconds() << "ms\n"
                  << "perlinOctaves:  " << perlinOctaves << "\n\n"
                  << "Use the arrow keys to change the values.\nUse the return key to regenerate the terrain.\n\n";

            for (std::size_t i = 0; i < settings.size(); ++i)
                osstr << ((i == currentSetting) ? ">>  " : "       ") << settings[i].name << ":  "
                      << *(settings[i].value) << '\n';

            hudText.setString(osstr.str());

            window.draw(hudText);
        }

        // Display things on screen
        window.display();
    }

    // Shut down our thread pool
    {
        const std::lock_guard lock(workQueueMutex);
        workPending = false;
    }

    while (!threads.empty())
    {
        threads.back().join();
        threads.pop_back();
    }
}


////////////////////////////////////////////////////////////
/// Get the terrain elevation at the given coordinates.
///
////////////////////////////////////////////////////////////
float getElevation(float x, float y)
{
    x = x / resolutionX - 0.5f;
    y = y / resolutionY - 0.5f;

    float elevation = 0.0f;

    for (int i = 0; i < perlinOctaves; ++i)
    {
        elevation += stb_perlin_noise3(x * perlinFrequency * static_cast<float>(std::pow(perlinFrequencyBase, i)),
                                       y * perlinFrequency * static_cast<float>(std::pow(perlinFrequencyBase, i)),
                                       0,
                                       0,
                                       0,
                                       0) *
                     static_cast<float>(std::pow(perlinFrequencyBase, -i));
    }

    elevation = (elevation + 1.f) / 2.f;

    const float distance = 2.0f * std::sqrt(x * x + y * y);
    elevation            = (elevation + heightBase) * (1.0f - edgeFactor * std::pow(distance, edgeDropoffExponent));
    elevation            = std::clamp(elevation, 0.0f, 1.0f);

    return elevation;
}

float getElevation(unsigned int x, unsigned int y)
{
    return getElevation(static_cast<float>(x), static_cast<float>(y));
}


////////////////////////////////////////////////////////////
/// Get the terrain moisture at the given coordinates.
///
////////////////////////////////////////////////////////////
float getMoisture(float x, float y)
{
    x = x / resolutionX - 0.5f;
    y = y / resolutionY - 0.5f;

    const float moisture = stb_perlin_noise3(x * 4.f + 0.5f, y * 4.f + 0.5f, 0, 0, 0, 0);

    return (moisture + 1.f) / 2.f;
}

float getMoisture(unsigned int x, unsigned int y)
{
    return getMoisture(static_cast<float>(x), static_cast<float>(y));
}


////////////////////////////////////////////////////////////
/// Get the lowlands terrain color for the given moisture.
///
////////////////////////////////////////////////////////////
sf::Color colorFromFloats(float r, float g, float b)
{
    return {static_cast<std::uint8_t>(r), static_cast<std::uint8_t>(g), static_cast<std::uint8_t>(b)};
}

sf::Color getLowlandsTerrainColor(float moisture)
{
    if (moisture < 0.27f)
        return colorFromFloats(240, 240, 180);
    if (moisture < 0.3f)
        return colorFromFloats(240 - (240 * (moisture - 0.27f) / 0.03f),
                               240 - (40 * (moisture - 0.27f) / 0.03f),
                               180 - (180 * (moisture - 0.27f) / 0.03f));
    if (moisture < 0.4f)
        return colorFromFloats(0, 200, 0);
    if (moisture < 0.48f)
        return colorFromFloats(0, 200 - (40 * (moisture - 0.4f) / 0.08f), 0);
    if (moisture < 0.6f)
        return colorFromFloats(0, 160, 0);
    if (moisture < 0.7f)
        return colorFromFloats((34 * (moisture - 0.6f) / 0.1f),
                               160 - (60 * (moisture - 0.6f) / 0.1f),
                               (34 * (moisture - 0.6f) / 0.1f));
    return colorFromFloats(34, 100, 34);
}


////////////////////////////////////////////////////////////
/// Get the highlands terrain color for the given elevation
/// and moisture.
///
////////////////////////////////////////////////////////////
sf::Color getHighlandsTerrainColor(float elevation, float moisture)
{
    const sf::Color lowlandsColor = getLowlandsTerrainColor(moisture);

    sf::Color color = moisture < 0.6f ? sf::Color(112, 128, 144)
                                      : colorFromFloats(112 + (110 * (moisture - 0.6f) / 0.4f),
                                                        128 + (56 * (moisture - 0.6f) / 0.4f),
                                                        144 - (9 * (moisture - 0.6f) / 0.4f));

    const float factor = std::min((elevation - 0.4f) / 0.1f, 1.f);

    color.r = static_cast<std::uint8_t>(lowlandsColor.r * (1.f - factor) + color.r * factor);
    color.g = static_cast<std::uint8_t>(lowlandsColor.g * (1.f - factor) + color.g * factor);
    color.b = static_cast<std::uint8_t>(lowlandsColor.b * (1.f - factor) + color.b * factor);

    return color;
}


////////////////////////////////////////////////////////////
/// Get the snowcap terrain color for the given elevation
/// and moisture.
///
////////////////////////////////////////////////////////////
sf::Color getSnowcapTerrainColor(float elevation, float moisture)
{
    const sf::Color highlandsColor = getHighlandsTerrainColor(elevation, moisture);

    sf::Color color = sf::Color::White;

    const float factor = std::min((elevation - snowcapHeight) / 0.05f, 1.f);

    color.r = static_cast<std::uint8_t>(highlandsColor.r * (1.f - factor) + color.r * factor);
    color.g = static_cast<std::uint8_t>(highlandsColor.g * (1.f - factor) + color.g * factor);
    color.b = static_cast<std::uint8_t>(highlandsColor.b * (1.f - factor) + color.b * factor);

    return color;
}


////////////////////////////////////////////////////////////
/// Get the terrain color for the given elevation and
/// moisture.
///
////////////////////////////////////////////////////////////
sf::Color getTerrainColor(float elevation, float moisture)
{
    if (elevation < 0.11f)
        return {0, 0, static_cast<std::uint8_t>(elevation / 0.11f * 74.f + 181.0f)};
    if (elevation < 0.14f)
        return {static_cast<std::uint8_t>(std::pow((elevation - 0.11f) / 0.03f, 0.3f) * 48.f),
                static_cast<std::uint8_t>(std::pow((elevation - 0.11f) / 0.03f, 0.3f) * 48.f),
                255};
    if (elevation < 0.16f)
        return {static_cast<std::uint8_t>((elevation - 0.14f) * 128.f / 0.02f + 48.f),
                static_cast<std::uint8_t>((elevation - 0.14f) * 128.f / 0.02f + 48.f),
                static_cast<std::uint8_t>(127.0f + (0.16f - elevation) * 128.f / 0.02f)};
    if (elevation < 0.17f)
        return {240, 230, 140};
    if (elevation < 0.4f)
        return getLowlandsTerrainColor(moisture);
    if (elevation < snowcapHeight)
        return getHighlandsTerrainColor(elevation, moisture);
    return getSnowcapTerrainColor(elevation, moisture);
}


////////////////////////////////////////////////////////////
/// Compute a compressed representation of the surface
/// normal based on the given coordinates, and the elevation
/// of the 4 adjacent neighbours.
///
////////////////////////////////////////////////////////////
sf::Vector2f computeNormal(float left, float right, float bottom, float top)
{
    const sf::Vector3f deltaX(1, 0, (std::pow(right, heightFlatten) - std::pow(left, heightFlatten)) * heightFactor);
    const sf::Vector3f deltaY(0, 1, (std::pow(top, heightFlatten) - std::pow(bottom, heightFlatten)) * heightFactor);

    sf::Vector3f crossProduct(deltaX.y * deltaY.z - deltaX.z * deltaY.y,
                              deltaX.z * deltaY.x - deltaX.x * deltaY.z,
                              deltaX.x * deltaY.y - deltaX.y * deltaY.x);

    // Scale cross product to make z component 1.0f so we can drop it
    crossProduct /= crossProduct.z;

    // Return "compressed" normal
    return {crossProduct.x, crossProduct.y};
}


////////////////////////////////////////////////////////////
/// Process a terrain generation work item. Use the vector
/// of vertices as scratch memory and upload the data to
/// the vertex buffer when done.
///
////////////////////////////////////////////////////////////
void processWorkItem(std::vector<sf::Vertex>& vertices, const WorkItem& workItem)
{
    const unsigned int rowBlockSize = (resolutionY / blockCount) + 1;
    const unsigned int rowStart     = rowBlockSize * workItem.index;

    if (rowStart >= resolutionY)
        return;

    const unsigned int rowEnd   = std::min(rowStart + rowBlockSize, resolutionY);
    const unsigned int rowCount = rowEnd - rowStart;

    const float scalingFactorX = static_cast<float>(windowWidth) / static_cast<float>(resolutionX);
    const float scalingFactorY = static_cast<float>(windowHeight) / static_cast<float>(resolutionY);

    for (unsigned int y = rowStart; y < rowEnd; ++y)
    {
        for (unsigned int x = 0; x < resolutionX; ++x)
        {
            const unsigned int arrayIndexBase = ((y - rowStart) * resolutionX + x) * 6;

            // Top left corner (first triangle)
            if (x > 0)
            {
                vertices[arrayIndexBase + 0] = vertices[arrayIndexBase - 6 + 5];
            }
            else if (y > rowStart)
            {
                vertices[arrayIndexBase + 0] = vertices[arrayIndexBase - resolutionX * 6 + 1];
            }
            else
            {
                vertices[arrayIndexBase + 0].position  = sf::Vector2f(static_cast<float>(x) * scalingFactorX,
                                                                     static_cast<float>(y) * scalingFactorY);
                vertices[arrayIndexBase + 0].color     = getTerrainColor(getElevation(x, y), getMoisture(x, y));
                vertices[arrayIndexBase + 0].texCoords = computeNormal(getElevation(x - 1, y),
                                                                       getElevation(x + 1, y),
                                                                       getElevation(x, y + 1),
                                                                       getElevation(x, y - 1));
            }

            // Bottom left corner (first triangle)
            if (x > 0)
            {
                vertices[arrayIndexBase + 1] = vertices[arrayIndexBase - 6 + 2];
            }
            else
            {
                vertices[arrayIndexBase + 1].position  = sf::Vector2f(static_cast<float>(x) * scalingFactorX,
                                                                     static_cast<float>(y + 1) * scalingFactorY);
                vertices[arrayIndexBase + 1].color     = getTerrainColor(getElevation(x, y + 1), getMoisture(x, y + 1));
                vertices[arrayIndexBase + 1].texCoords = computeNormal(getElevation(x - 1, y + 1),
                                                                       getElevation(x + 1, y + 1),
                                                                       getElevation(x, y + 2),
                                                                       getElevation(x, y));
            }

            // Bottom right corner (first triangle)
            vertices[arrayIndexBase + 2].position = sf::Vector2f(static_cast<float>(x + 1) * scalingFactorX,
                                                                 static_cast<float>(y + 1) * scalingFactorY);
            vertices[arrayIndexBase + 2].color = getTerrainColor(getElevation(x + 1, y + 1), getMoisture(x + 1, y + 1));
            vertices[arrayIndexBase + 2].texCoords = computeNormal(getElevation(x, y + 1),
                                                                   getElevation(x + 2, y + 1),
                                                                   getElevation(x + 1, y + 2),
                                                                   getElevation(x + 1, y));

            // Top left corner (second triangle)
            vertices[arrayIndexBase + 3] = vertices[arrayIndexBase + 0];

            // Bottom right corner (second triangle)
            vertices[arrayIndexBase + 4] = vertices[arrayIndexBase + 2];

            // Top right corner (second triangle)
            if (y > rowStart)
            {
                vertices[arrayIndexBase + 5] = vertices[arrayIndexBase - resolutionX * 6 + 2];
            }
            else
            {
                vertices[arrayIndexBase + 5].position  = sf::Vector2f(static_cast<float>(x + 1) * scalingFactorX,
                                                                     static_cast<float>(y) * scalingFactorY);
                vertices[arrayIndexBase + 5].color     = getTerrainColor(getElevation(x + 1, y), getMoisture(x + 1, y));
                vertices[arrayIndexBase + 5].texCoords = computeNormal(getElevation(x, y),
                                                                       getElevation(x + 2, y),
                                                                       getElevation(x + 1, y + 1),
                                                                       getElevation(x + 1, y - 1));
            }
        }
    }

    // Copy the resulting geometry from our thread-local buffer into the target buffer
    std::memcpy(workItem.targetBuffer + (resolutionX * rowStart * 6),
                vertices.data(),
                sizeof(sf::Vertex) * resolutionX * rowCount * 6);
}


////////////////////////////////////////////////////////////
/// Worker thread entry point. We use a thread pool to avoid
/// the heavy cost of constantly recreating and starting
/// new threads whenever we need to regenerate the terrain.
///
////////////////////////////////////////////////////////////
void threadFunction()
{
    const unsigned int rowBlockSize = (resolutionY / blockCount) + 1;

    std::vector<sf::Vertex> vertices(resolutionX * rowBlockSize * 6);

    WorkItem workItem = {nullptr, 0};

    // Loop until the application exits
    for (;;)
    {
        workItem.targetBuffer = nullptr;

        // Check if there are new work items in the queue
        {
            const std::lock_guard lock(workQueueMutex);

            if (!workPending)
                return;

            if (!workQueue.empty())
            {
                workItem = workQueue.front();
                workQueue.pop_front();
            }
        }

        // If we didn't receive a new work item, keep looping
        if (!workItem.targetBuffer)
        {
            sf::sleep(sf::milliseconds(10));

            continue;
        }

        processWorkItem(vertices, workItem);

        {
            const std::lock_guard lock(workQueueMutex);

            --pendingWorkCount;
        }
    }
}


////////////////////////////////////////////////////////////
/// Terrain generation entry point. This queues up the
/// generation work items which the worker threads dequeue
/// and process.
///
////////////////////////////////////////////////////////////
void generateTerrain(sf::Vertex* buffer)
{
    bufferUploadPending = true;

    // Make sure the work queue is empty before queuing new work
    for (;;)
    {
        {
            const std::lock_guard lock(workQueueMutex);

            if (workQueue.empty())
                break;
        }

        sf::sleep(sf::milliseconds(10));
    }

    // Queue all the new work items
    {
        const std::lock_guard lock(workQueueMutex);

        for (unsigned int i = 0; i < blockCount; ++i)
        {
            const WorkItem workItem = {buffer, i};
            workQueue.push_back(workItem);
        }

        pendingWorkCount = blockCount;
    }
}
