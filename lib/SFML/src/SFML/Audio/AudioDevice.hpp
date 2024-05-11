////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio/Listener.hpp>

#include <SFML/System/Vector3.hpp>

#include <miniaudio.h>

#include <optional>


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief High-level wrapper around the audio API, it manages
///        the creation and destruction of the audio device and
///        context and stores the device capabilities
///
////////////////////////////////////////////////////////////
class AudioDevice
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    AudioDevice();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~AudioDevice();

    ////////////////////////////////////////////////////////////
    /// \brief Get the audio engine
    ///
    /// There should only be a single instance of AudioDevice.
    /// As long as an AudioResource exists, this function should
    /// always return a valid pointer to the audio engine.
    ///
    /// \return The audio engine
    ///
    ////////////////////////////////////////////////////////////
    static ma_engine* getEngine();

    ////////////////////////////////////////////////////////////
    /// \brief Change the global volume of all the sounds and musics
    ///
    /// The volume is a number between 0 and 100; it is combined with
    /// the individual volume of each sound / music.
    /// The default value for the volume is 100 (maximum).
    ///
    /// \param volume New global volume, in the range [0, 100]
    ///
    /// \see getGlobalVolume
    ///
    ////////////////////////////////////////////////////////////
    static void setGlobalVolume(float volume);

    ////////////////////////////////////////////////////////////
    /// \brief Get the current value of the global volume
    ///
    /// \return Current global volume, in the range [0, 100]
    ///
    /// \see setGlobalVolume
    ///
    ////////////////////////////////////////////////////////////
    static float getGlobalVolume();

    ////////////////////////////////////////////////////////////
    /// \brief Set the position of the listener in the scene
    ///
    /// The default listener's position is (0, 0, 0).
    ///
    /// \param position New listener's position
    ///
    /// \see getPosition, setDirection
    ///
    ////////////////////////////////////////////////////////////
    static void setPosition(const Vector3f& position);

    ////////////////////////////////////////////////////////////
    /// \brief Get the current position of the listener in the scene
    ///
    /// \return Listener's position
    ///
    /// \see setPosition
    ///
    ////////////////////////////////////////////////////////////
    static Vector3f getPosition();

    ////////////////////////////////////////////////////////////
    /// \brief Set the forward vector of the listener in the scene
    ///
    /// The direction (also called "at vector") is the vector
    /// pointing forward from the listener's perspective. Together
    /// with the up vector, it defines the 3D orientation of the
    /// listener in the scene. The direction vector doesn't
    /// have to be normalized.
    /// The default listener's direction is (0, 0, -1).
    ///
    /// \param direction New listener's direction
    ///
    /// \see getDirection, setUpVector, setPosition
    ///
    ////////////////////////////////////////////////////////////
    static void setDirection(const Vector3f& direction);

    ////////////////////////////////////////////////////////////
    /// \brief Get the current forward vector of the listener in the scene
    ///
    /// \return Listener's forward vector (not normalized)
    ///
    /// \see setDirection
    ///
    ////////////////////////////////////////////////////////////
    static Vector3f getDirection();

    ////////////////////////////////////////////////////////////
    /// \brief Set the velocity of the listener in the scene
    ///
    /// The default listener's velocity is (0, 0, -1).
    ///
    /// \param velocity New listener's velocity
    ///
    /// \see getVelocity, getDirection, setUpVector, setPosition
    ///
    ////////////////////////////////////////////////////////////
    static void setVelocity(const Vector3f& velocity);

    ////////////////////////////////////////////////////////////
    /// \brief Get the current forward vector of the listener in the scene
    ///
    /// \return Listener's velocity
    ///
    /// \see setVelocity
    ///
    ////////////////////////////////////////////////////////////
    static Vector3f getVelocity();

    ////////////////////////////////////////////////////////////
    /// \brief Set the cone properties of the listener in the audio scene
    ///
    /// The cone defines how directional attenuation is applied.
    /// The default cone of a sound is {2 * PI, 2 * PI, 1}.
    ///
    /// \param cone Cone properties of the listener in the scene
    ///
    /// \see getCone
    ///
    ////////////////////////////////////////////////////////////
    static void setCone(const Listener::Cone& cone);

    ////////////////////////////////////////////////////////////
    /// \brief Get the cone properties of the listener in the audio scene
    ///
    /// \return Cone properties of the listener
    ///
    /// \see setCone
    ///
    ////////////////////////////////////////////////////////////
    static Listener::Cone getCone();

    ////////////////////////////////////////////////////////////
    /// \brief Set the upward vector of the listener in the scene
    ///
    /// The up vector is the vector that points upward from the
    /// listener's perspective. Together with the direction, it
    /// defines the 3D orientation of the listener in the scene.
    /// The up vector doesn't have to be normalized.
    /// The default listener's up vector is (0, 1, 0). It is usually
    /// not necessary to change it, especially in 2D scenarios.
    ///
    /// \param upVector New listener's up vector
    ///
    /// \see getUpVector, setDirection, setPosition
    ///
    ////////////////////////////////////////////////////////////
    static void setUpVector(const Vector3f& upVector);

    ////////////////////////////////////////////////////////////
    /// \brief Get the current upward vector of the listener in the scene
    ///
    /// \return Listener's upward vector (not normalized)
    ///
    /// \see setUpVector
    ///
    ////////////////////////////////////////////////////////////
    static Vector3f getUpVector();

private:
    ////////////////////////////////////////////////////////////
    /// \brief This function makes sure the instance pointer is initialized before using it
    ///
    /// \return The instance pointer
    ///
    ////////////////////////////////////////////////////////////
    static AudioDevice*& getInstance();

    struct ListenerProperties
    {
        float          volume{100.f};
        sf::Vector3f   position{0, 0, 0};
        sf::Vector3f   direction{0, 0, -1};
        sf::Vector3f   velocity{0, 0, 0};
        Listener::Cone cone{sf::degrees(360), sf::degrees(360), 1};
        sf::Vector3f   upVector{0, 1, 0};
    };

    ////////////////////////////////////////////////////////////
    /// \brief Get the listener properties
    ///
    /// \return The listener properties
    ///
    ////////////////////////////////////////////////////////////
    static ListenerProperties& getListenerProperties();

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    std::optional<ma_log>     m_log;            //!< The miniaudio log
    std::optional<ma_context> m_context;        //!< The miniaudio context
    std::optional<ma_device>  m_playbackDevice; //!< The miniaudio playback device
    std::optional<ma_engine>  m_engine;         //!< The miniaudio engine (used for effects and spatialisation)
};

} // namespace sf::priv
