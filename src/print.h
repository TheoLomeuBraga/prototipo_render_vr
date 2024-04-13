#pragma once
#include <iostream> // Para std::ostream e std::cout
#include <variant>  // Para std::variant
#include <string>   // Para std::string


struct printable_any
{
	std::variant<std::string, int, double> m_val;

	template <class... Args>
	printable_any(Args &&...args) : m_val(std::forward<Args>(args)...)
	{
	}

	friend std::ostream &operator<<(std::ostream &os, const printable_any &val)
	{
		std::visit([&os](const auto &value)
				   { os << value; },
				   val.m_val);
		return os;
	}
};

template <typename... Args>
void print(const Args &...args)
{
	((std::cout << args << " "), ...);
	std::cout << '\n';
}