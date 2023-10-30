#pragma once

#include "bump_io.hpp"
#include "bump_log.hpp"
#include "bump_range.hpp"

#include <map>
#include <string>
#include <vector>

namespace bump
{

	namespace io
	{
	
#pragma region std::pair

		template<class T1, class T2>
		struct write_impl<std::pair<T1, T2>>
		{
			static void write(std::ostream& os, std::pair<T1, T2> const& value, std::endian endian)
			{
				write<T1>(os, value.first, endian);
				write<T2>(os, value.second, endian);
			}
		};

		template<class T1, class T2>
		struct read_impl<std::pair<T1, T2>>
		{
			static std::pair<T1, T2> read(std::istream& is, std::endian endian)
			{
				auto first = read<T1>(is, endian);
				auto second = read<T2>(is, endian);
				return { std::move(first), std::move(second) };
			}
		};

#pragma endregion

#pragma region std::map

		template<class Key, class T, class Comp, class Alloc>
		struct write_impl<std::map<Key, T, Comp, Alloc>>
		{
			static void write(std::ostream& os, std::map<Key, T, Comp, Alloc> const& value, std::endian endian)
			{
				write<std::uint64_t>(os, value.size(), endian);

				for (auto const& p : value)
					write<std::map<Key, T, Comp, Alloc>::value_type>(os, p, endian);
			}
		};

		template<class Key, class T, class Comp, class Alloc>
		struct read_impl<std::map<Key, T, Comp, Alloc>>
		{
			static std::map<Key, T, Comp, Alloc> read(std::istream& is, std::endian endian)
			{
				auto const size = read<std::uint64_t>(is, endian);

				if (size > std::map<Key, T, Comp, Alloc>::max_size())
				{
					log_error("bump::io::read_impl<std::map<Key, T, Comp, Alloc>>::read() failed: map size too large for this platform!");
					is.setstate(std::ios::failbit);
					return { };
				}

				auto result = std::map<Key, T, Comp, Alloc>();

				for (auto i : range(0, size))
				{
					auto p = read<std::map<Key, T, Comp, Alloc>::value_type>(is, endian);
					auto const it = result.insert(std::move(p));

					if (!it.second)
					{
						log_error("bump::io::read_impl<std::map<Key, T, Comp, Alloc>>::read() failed: duplicate map key!");
						is.setstate(std::ios::failbit);
						return { };
					}
				}

				return result;
			}
		};

#pragma endregion

#pragma region std::string

		template<class CharT, class Traits, class Alloc>
		struct write_impl<std::basic_string<CharT, Traits, Alloc>>
		{
			static void write(std::ostream& os, std::basic_string<CharT, Traits, Alloc> const& value, std::endian endian)
			{
				write<std::uint64_t>(os, value.size(), endian);

				for (auto const& item : value)
					write<CharT>(os, item, endian);
			}
		};

		template<class CharT, class Traits, class Alloc>
		struct read_impl<std::basic_string<CharT, Traits, Alloc>>
		{
			static std::basic_string<CharT, Traits, Alloc> read(std::istream& is, std::endian endian)
			{
				auto const size = read<std::uint64_t>(is, endian);

				if (size > std::basic_string<CharT, Traits, Alloc>::max_size())
				{
					log_error("bump::io::read_impl<std::basic_string<CharT, Traits, Alloc>>::read() failed: string size too large for this platform!");
					is.setstate(std::ios::failbit);
					return { };
				}

				auto result = std::basic_string<CharT, Traits, Alloc>();
				result.reserve(size);

				for (auto i : range(0, size))
					result.push_back(read<CharT>(is, endian));

				return result;
			}
		};

#pragma endregion

#pragma region std::vector

		template<class T, class Alloc>
		struct write_impl<std::vector<T, Alloc>>
		{
			static void write(std::ostream& os, std::vector<T, Alloc> const& value, std::endian endian)
			{
				write<std::uint64_t>(os, value.size(), endian); 

				for (auto const& item : value)
					write<T>(os, item, endian);
			}
		};

		template<class T, class Alloc>
		struct read_impl<std::vector<T, Alloc>>
		{
			static std::vector<T, Alloc> read(std::istream& is, std::endian endian)
			{
				auto const size = read<std::uint64_t>(is, endian);

				if (size > std::vector<T, Alloc>::max_size())
				{
					log_error("bump::io::read_impl<std::vector<T, Alloc>>::read() failed: vector size too large for this platform!");
					is.setstate(std::ios::failbit);
					return { };
				}

				auto result = std::vector<T, Alloc>();
				result.reserve(size);

				for (auto i : range(0, size))
					result.push_back(read<T>(is, endian));

				return result;
			}
		};

#pragma endregion

	} // io

} // bump
