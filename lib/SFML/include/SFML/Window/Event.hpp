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
#include <SFML/Config.hpp>

#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Sensor.hpp>

#include <SFML/System/Vector2.hpp>

#include <variant>


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Defines a system event and its parameters
///
////////////////////////////////////////////////////////////
class SFML_WINDOW_API Event
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Empty event
    ///
    ////////////////////////////////////////////////////////////
    struct Empty
    {
    };

    ////////////////////////////////////////////////////////////
    /// \brief Closed event
    ///
    ////////////////////////////////////////////////////////////
    struct Closed
    {
    };

    ////////////////////////////////////////////////////////////
    /// \brief Resized event
    ///
    ////////////////////////////////////////////////////////////
    struct Resized
    {
        Vector2u size; //!< New size, in pixels
    };

    ////////////////////////////////////////////////////////////
    /// \brief Lost focus event
    ///
    ////////////////////////////////////////////////////////////
    struct FocusLost
    {
    };

    ////////////////////////////////////////////////////////////
    /// \brief Gained focus event
    ///
    ////////////////////////////////////////////////////////////
    struct FocusGained
    {
    };

    ////////////////////////////////////////////////////////////
    /// \brief Text event
    ///
    ////////////////////////////////////////////////////////////
    struct TextEntered
    {
        std::uint32_t unicode{}; //!< UTF-32 Unicode value of the character
    };

    ////////////////////////////////////////////////////////////
    /// \brief KeyChanged events
    ///
    ////////////////////////////////////////////////////////////
    struct KeyChanged
    {
        Keyboard::Key      code{};     //!< Code of the key
        Keyboard::Scancode scancode{}; //!< Physical code of the key
        bool               alt{};      //!< Is the Alt key pressed?
        bool               control{};  //!< Is the Control key pressed?
        bool               shift{};    //!< Is the Shift key pressed?
        bool               system{};   //!< Is the System key pressed?
    };
    struct KeyPressed : KeyChanged
    {
    };
    struct KeyReleased : KeyChanged
    {
    };

    ////////////////////////////////////////////////////////////
    /// \brief Mouse wheel scrolled event
    ///
    ////////////////////////////////////////////////////////////
    struct MouseWheelScrolled
    {
        Mouse::Wheel wheel{}; //!< Which wheel (for mice with multiple ones)
        float        delta{}; //!< Wheel offset (positive is up/left, negative is down/right). High-precision mice may use non-integral offsets.
        Vector2i     position; //!< Position of the mouse pointer, relative to the top left of the owner window
    };

    ////////////////////////////////////////////////////////////
    /// \brief Mouse button changed events
    ///
    ////////////////////////////////////////////////////////////
    struct MouseButtonChanged
    {
        Mouse::Button button{}; //!< Code of the button that has been pressed
        Vector2i      position; //!< Position of the mouse pointer, relative to the top left of the owner window
    };
    struct MouseButtonPressed : MouseButtonChanged
    {
    };
    struct MouseButtonReleased : MouseButtonChanged
    {
    };

    ////////////////////////////////////////////////////////////
    /// \brief Mouse move event
    ///
    ////////////////////////////////////////////////////////////
    struct MouseMoved
    {
        Vector2i position; //!< Position of the mouse pointer, relative to the top left of the owner window
    };

    ////////////////////////////////////////////////////////////
    /// \brief Mouse entered event
    ///
    ////////////////////////////////////////////////////////////
    struct MouseEntered
    {
    };

    ////////////////////////////////////////////////////////////
    /// \brief Mouse left event
    ///
    ////////////////////////////////////////////////////////////
    struct MouseLeft
    {
    };

    ////////////////////////////////////////////////////////////
    /// \brief Joystick button events
    ///
    ////////////////////////////////////////////////////////////
    struct JoystickButtonChanged
    {
        unsigned int joystickId{}; //!< Index of the joystick (in range [0 .. Joystick::Count - 1])
        unsigned int button{}; //!< Index of the button that has been pressed (in range [0 .. Joystick::ButtonCount - 1])
    };
    struct JoystickButtonPressed : JoystickButtonChanged
    {
    };
    struct JoystickButtonReleased : JoystickButtonChanged
    {
    };

    ////////////////////////////////////////////////////////////
    /// \brief Joystick axis move event
    ///
    ////////////////////////////////////////////////////////////
    struct JoystickMoved
    {
        unsigned int   joystickId{}; //!< Index of the joystick (in range [0 .. Joystick::Count - 1])
        Joystick::Axis axis{};       //!< Axis on which the joystick moved
        float          position{};   //!< New position on the axis (in range [-100 .. 100])
    };

    ////////////////////////////////////////////////////////////
    /// \brief Joystick connection events
    ///
    ////////////////////////////////////////////////////////////
    struct JoystickChanged
    {
        unsigned int joystickId{}; //!< Index of the joystick (in range [0 .. Joystick::Count - 1])
    };
    struct JoystickConnected : JoystickChanged
    {
    };
    struct JoystickDisconnected : JoystickChanged
    {
    };

    ////////////////////////////////////////////////////////////
    /// \brief Touch events
    ///
    ////////////////////////////////////////////////////////////
    struct TouchChanged
    {
        unsigned int finger{}; //!< Index of the finger in case of multi-touch events
        Vector2i     position; //!< Position of the touch, relative to the top left of the owner window
    };
    struct TouchBegan : TouchChanged
    {
    };
    struct TouchMoved : TouchChanged
    {
    };
    struct TouchEnded : TouchChanged
    {
    };

    ////////////////////////////////////////////////////////////
    /// \brief Sensor event
    ///
    ////////////////////////////////////////////////////////////
    struct SensorChanged
    {
        Sensor::Type type{}; //!< Type of the sensor
        Vector3f     value;  //!< Current value of the sensor on the X, Y, and Z axes
    };

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// Sets the event to sf::Event::Empty
    ///
    ////////////////////////////////////////////////////////////
    Event() = default;

    ////////////////////////////////////////////////////////////
    /// \brief Construct from a given sf::Event subtype
    ///
    /// \param t Event subtype
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    Event(const T& t);

    ////////////////////////////////////////////////////////////
    /// \brief Check current event type
    ///
    /// \return True if template parameter is current event type
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] bool is() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get particular event type
    ///
    /// \return Address of current event type, otherwise nullptr
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] const T* getIf() const;

    ////////////////////////////////////////////////////////////
    /// \brief Check if current event type is not `Empty`
    ///
    /// \return True if current event type is not `Empty`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit operator bool() const
    {
        return !is<Empty>();
    }

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    std::variant<Empty,
                 Closed,
                 Resized,
                 FocusLost,
                 FocusGained,
                 TextEntered,
                 KeyPressed,
                 KeyReleased,
                 MouseWheelScrolled,
                 MouseButtonPressed,
                 MouseButtonReleased,
                 MouseMoved,
                 MouseEntered,
                 MouseLeft,
                 JoystickButtonPressed,
                 JoystickButtonReleased,
                 JoystickMoved,
                 JoystickConnected,
                 JoystickDisconnected,
                 TouchBegan,
                 TouchMoved,
                 TouchEnded,
                 SensorChanged>
        m_data; //!< Event data

    ////////////////////////////////////////////////////////////
    // Helper functions
    ////////////////////////////////////////////////////////////
    template <typename T, typename... Ts>
    static constexpr bool isInParameterPack(const std::variant<Ts...>&)
    {
        return (std::is_same_v<T, Ts> || ...);
    }

    template <typename T>
    static constexpr bool isEventType = isInParameterPack<T>(decltype(m_data)());
};

} // namespace sf

#include <SFML/Window/Event.inl>


////////////////////////////////////////////////////////////
/// \class sf::Event
/// \ingroup window
///
/// sf::Event holds all the information about a system event
/// that just happened. Events are retrieved using the
/// sf::Window::pollEvent and sf::Window::waitEvent functions.
///
/// A sf::Event instance contains the type of the event
/// (mouse moved, key pressed, window closed, ...) as well
/// as the details about this particular event. Each event
/// corresponds to a different struct which contains the data
/// required to process that event.
///
/// The way to access the current active event is via
/// sf::Event::getIf. This member function returns the address
/// of the event struct if the event type matches the active
/// event, otherise it returns nullptr. sf::Event::is is used
/// to check the active event type without actually reading
/// any of the corresponding event data.
///
/// \code
/// while (const auto event = window.pollEvent())
/// {
///     // Request for closing the window
///     if (event.is<sf::Event::Closed>())
///         window.close();
///
///     // The escape key was pressed
///     if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
///         if (keyPressed->code == sf::Keyboard::Key::Escape)
///             window.close();
///
///     // The window was resized
///     if (const auto* resized = event.getIf<sf::Event::Resized>())
///         doSomethingWithTheNewSize(resized->size);
///
///     // etc ...
/// }
/// \endcode
///
////////////////////////////////////////////////////////////
