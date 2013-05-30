#ifndef NUMBER_FORMAT_HPP
#define NUMBER_FORMAT_HPP

#include <iostream>
#include <locale>

namespace number_format
{
	template <typename T>
	struct ForceThousandsSep
		: std::numpunct<T>
	{
		virtual std::basic_string<T> do_grouping() const { return "\3"; }
		virtual T do_thousands_sep() const { return ','; }
		virtual T do_decimal_point() const { return '.'; }
	};

	void	set_output_number_format()
	{
		std::cout.imbue(std::locale(std::locale(""), new ForceThousandsSep<char>()));
	}
};

#endif // !NUMBER_FORMAT_HPP