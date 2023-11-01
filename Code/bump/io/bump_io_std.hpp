#pragma once

#include "bump_io.hpp"
#include "bump_log.hpp"
#include "bump_range.hpp"

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace bump
{

	namespace io
	{
	
#pragma region std::pair

		template<class T1, class T2>
		struct write_impl<std::pair<T1, T2>>
		{
			static void write(std::ostream& os, std::pair<T1, T2> const& value)
			{
				io::write<T1>(os, value.first);
				io::write<T2>(os, value.second);
			}
		};

		template<class T1, class T2>
		struct read_impl<std::pair<T1, T2>>
		{
			static std::pair<T1, T2> read(std::istream& is)
			{
				auto first = io::read<T1>(is);
				auto second = io::read<T2>(is);
				return { std::move(first), std::move(second) };
			}
		};

#pragma endregion

#pragma region std::map

		template<class Key, class T, class Comp, class Alloc>
		struct write_impl<std::map<Key, T, Comp, Alloc>>
		{
			static void write(std::ostream& os, std::map<Key, T, Comp, Alloc> const& value)
			{
				io::write<std::uint64_t>(os, value.size());

				for (auto const& p : value)
					io::write<std::map<Key, T, Comp, Alloc>::value_type>(os, p);
			}
		};

		template<class Key, class T, class Comp, class Alloc>
		struct read_impl<std::map<Key, T, Comp, Alloc>>
		{
			static std::map<Key, T, Comp, Alloc> read(std::istream& is)
			{
				auto result = std::map<Key, T, Comp, Alloc>();

				auto const size = io::read<std::uint64_t>(is);

				if (size > result.max_size())
				{
					log_error("bump::io::read_impl<std::map<Key, T, Comp, Alloc>>::read() failed: map size too large for this platform!");
					is.setstate(std::ios::failbit);
					return { };
				}

				for ([[maybe_unused]] auto _ : range(0, size))
				{
					auto p = io::read<std::map<Key, T, Comp, Alloc>::value_type>(is);
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
			static void write(std::ostream& os, std::basic_string<CharT, Traits, Alloc> const& value)
			{
				io::write<std::uint64_t>(os, value.size());

				for (auto const& item : value)
					io::write<CharT>(os, item);
			}
		};

		template<class CharT, class Traits>
		struct write_impl<std::basic_string_view<CharT, Traits>>
		{
			static void write(std::ostream& os, std::basic_string_view<CharT, Traits> const& value)
			{
				io::write<std::uint64_t>(os, value.size());

				for (auto const& item : value)
					io::write<CharT>(os, item);
			}
		};

		template<class CharT, class Traits, class Alloc>
		struct read_impl<std::basic_string<CharT, Traits, Alloc>>
		{
			static std::basic_string<CharT, Traits, Alloc> read(std::istream& is)
			{
				auto result = std::basic_string<CharT, Traits, Alloc>();

				auto const size = io::read<std::uint64_t>(is);

				if (size > result.max_size())
				{
					log_error("bump::io::read_impl<std::basic_string<CharT, Traits, Alloc>>::read() failed: string size too large for this platform!");
					is.setstate(std::ios::failbit);
					return { };
				}

				result.reserve(size);

				for ([[maybe_unused]] auto _ : range(0, size))
					result.push_back(io::read<CharT>(is));

				return result;
			}
		};

#pragma endregion

#pragma region std::vector

		template<class T, class Alloc>
		struct write_impl<std::vector<T, Alloc>>
		{
			static void write(std::ostream& os, std::vector<T, Alloc> const& value)
			{
				io::write<std::uint64_t>(os, value.size());

				for (auto const& item : value)
					io::write<T>(os, item);
			}
		};

		template<class T, class Alloc>
		struct read_impl<std::vector<T, Alloc>>
		{
			static std::vector<T, Alloc> read(std::istream& is)
			{
				auto result = std::vector<T, Alloc>();

				auto const size = io::read<std::uint64_t>(is);

				if (size > result.max_size())
				{
					log_error("bump::io::read_impl<std::vector<T, Alloc>>::read() failed: vector size too large for this platform!");
					is.setstate(std::ios::failbit);
					return { };
				}

				result.reserve(size);

				for ([[maybe_unused]] auto _ : range(0, size))
					result.push_back(io::read<T>(is));

				return result;
			}
		};

#pragma endregion

	} // io

} // bump
