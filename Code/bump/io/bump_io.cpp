#include "bump_io.hpp"

#include "bump_die.hpp"

namespace bump
{

	namespace io
	{
	
		namespace detail
		{
		
			int endian_index =  std::ios_base::xalloc();
			
			void setendian_impl(std::ios_base& s, std::endian endian)
			{
				s.iword(detail::endian_index) = (endian == std::endian::little ? 1L : 0L);
			}

			std::endian getendian_impl(std::ios_base& s)
			{
				return (s.iword(detail::endian_index) == 1L ? std::endian::little : std::endian::big);
			}

		} // detail
		
		detail::stream_manipulator<std::endian> setendian(std::endian endian)
		{
			return detail::stream_manipulator<std::endian>(detail::setendian_impl, endian);
		}

		std::endian getendian(std::ios_base& s)
		{
			return detail::getendian_impl(s);
		}
	
	} // io

} // bump
