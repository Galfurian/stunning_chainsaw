/// @file stepper_improved_euler.hpp
/// @author Enrico Fraccaroli (enry.frak@gmail.com)
/// @brief Simplification of the code available at:
///     https://github.com/headmyshoulder/odeint-v2

#pragma once

#include "chainsaw/detail/type_traits.hpp"
#include "chainsaw/detail/it_algebra.hpp"

namespace chainsaw
{

/// @brief Stepper implementing Heun's method for numerical integration (also
/// known as the Improved Euler Method).
/// @tparam State The state vector type.
/// @tparam Time The datatype used to hold time.
template <class State, class Time>
class stepper_improved_euler {
public:
    /// @brief Type used for the order of the stepper.
    using order_type = unsigned short;

    /// @brief Type used to keep track of time.
    using time_type = Time;

    /// @brief The state vector type.
    using state_type = State;

    /// @brief Type of value contained in the state vector.
    using value_type = typename state_type::value_type;

    /// @brief Indicates whether this is an adaptive stepper.
    static constexpr bool is_adaptive_stepper = false;

    /// @brief Constructs a new stepper.
    stepper_improved_euler()
        : m_dxdt1(), ///< Initializes the first derivative state vector.
          m_dxdt2(), ///< Initializes the second derivative state vector.
          m_x(),     ///< Initializes the temporary state vector for intermediate calculations.
          m_steps()  ///< Initializes the step count.
    {
        // Nothing to do.
    }

    /// @brief Deleted copy constructor.
    stepper_improved_euler(const stepper_improved_euler &other) = delete;

    /// @brief Deleted copy assignment operator.
    stepper_improved_euler &operator=(const stepper_improved_euler &other) = delete;

    /// @brief Returns the order of the stepper.
    /// @return The order of the internal stepper, which is 2 for the Improved Euler method.
    constexpr inline order_type order_step() const
    {
        return 1;
    }

    /// @brief Adjusts the size of the internal state vectors based on a reference.
    /// @param reference A reference state vector used for size adjustment.
    void adjust_size(const state_type &reference)
    {
        if constexpr (chainsaw::detail::has_resize<state_type>::value) {
            m_dxdt1.resize(reference.size()); // Resize m_dxdt1 if supported.
            m_dxdt2.resize(reference.size()); // Resize m_dxdt2 if supported.
            m_x.resize(reference.size());     // Resize m_x if supported.
        }
    }

    /// @brief Returns the number of steps executed by the stepper so far.
    /// @return The number of integration steps executed.
    constexpr inline auto steps() const
    {
        return m_steps;
    }

    /// @brief Performs a single integration step using Heun's method (Improved Euler method).
    /// @param system The system to integrate.
    /// @param x The initial state vector.
    /// @param t The initial time.
    /// @param dt The time step for integration.
    template <class System>
    constexpr void do_step(System &&system, state_type &x, const time_type t, const time_type dt) noexcept
    {
        // Calculate the derivative at the initial point:
        //      dxdt1 = system(x, t);
        system(x, m_dxdt1, t);

        // Calculate the state at the next time point using Euler's method:
        //      m_x(t + dt) = x(t) + dxdt1 * dt;
        detail::it_algebra::scale_two_sum(m_x.begin(), m_x.end(), 1., x.begin(), dt, m_dxdt1.begin());

        // Calculate the derivative at the midpoint:
        //      dxdt2 = system(m_x, t + dt);
        system(m_x, m_dxdt2, t + dt);

        // Update the state vector using the average of the derivatives:
        //      x(t + dt) = x(t) + (dt / 2) * (dxdt1 + dxdt2);
        detail::it_algebra::scale_two_sum_accumulate(
            x.begin(), x.end(),
            dt * .5, m_dxdt1.begin(),
            dt * .5, m_dxdt2.begin());

        // Increment the number of integration steps.
        ++m_steps;
    }

private:
    /// Keeps track of the first derivative of the state.
    state_type m_dxdt1;

    /// Keeps track of the second derivative of the state.
    state_type m_dxdt2;

    /// Temporary state vector for intermediate calculations.
    state_type m_x;

    /// The number of steps taken during integration.
    unsigned long m_steps;
};

} // namespace chainsaw
