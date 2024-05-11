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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio/OutputSoundFile.hpp>
#include <SFML/Audio/SoundFileFactory.hpp>
#include <SFML/Audio/SoundFileWriter.hpp>


namespace sf
{
////////////////////////////////////////////////////////////
bool OutputSoundFile::openFromFile(const std::filesystem::path&     filename,
                                   unsigned int                     sampleRate,
                                   unsigned int                     channelCount,
                                   const std::vector<SoundChannel>& channelMap)
{
    // If the file is already open, first close it
    close();

    // Find a suitable writer for the file type
    m_writer = SoundFileFactory::createWriterFromFilename(filename);
    if (!m_writer)
        return false;

    // Pass the stream to the reader
    if (!m_writer->open(filename, sampleRate, channelCount, channelMap))
    {
        close();
        return false;
    }

    return true;
}


////////////////////////////////////////////////////////////
void OutputSoundFile::write(const std::int16_t* samples, std::uint64_t count)
{
    if (m_writer && samples && count)
        m_writer->write(samples, count);
}


////////////////////////////////////////////////////////////
void OutputSoundFile::close()
{
    // Destroy the reader
    m_writer.reset();
}

} // namespace sf
