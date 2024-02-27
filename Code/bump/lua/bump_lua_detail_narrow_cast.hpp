#pragma once

#include <concepts>
#include <limits>
#include <type_traits>
#include <stdexcept>

namespace bump
{

	namespace lua
	{

		namespace detail
		{

			template<class T, class U> requires std::is_integral_v<T> && std::is_integral_v<U>
			T narrow_cast(U u)
			{
				if constexpr (std::is_same_v<T, U>)
				{
					return u;
				}
				else
				{
					auto const t = static_cast<T>(u);

					if (static_cast<U>(t) != u)
						throw std::runtime_error("narrow_cast: value out of range");

					if constexpr (std::is_signed_v<T> != std::is_signed_v<U>)
						if ((t < T{ 0 }) != (u < U{ 0 }))
							throw std::runtime_error("narrow_cast: value out of range");

					return t;
				}
			}

			template<class T, class U> requires std::is_floating_point_v<T> && std::is_floating_point_v<U>
			T narrow_cast(U u)
			{
				if constexpr (std::is_same_v<T, U>)
				{
					return u;
				}
				else
				{
					if (u > std::numeric_limits<T>::max() || u < std::numeric_limits<T>::lowest())
						throw std::runtime_error("narrow_cast: value out of range");
					
					return static_cast<T>(u);
				}
			}

		} // detail

	} // lua

} // bump
