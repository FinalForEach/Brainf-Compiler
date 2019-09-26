#pragma once
#include <map>
#include <optional>
class Environment 
{
	std::optional<int> knownCellValue;
	public:
	std::map<int,int> knownCells;
	Environment()
	:knownCellValue(0)
	{
		for(int i=0;i<30000;i++)
		{
			knownCells[i]=0;//Initialize
		}
	}
	bool hasKnownCellValue() const
	{
		return knownCellValue.has_value();
	}
	int getKnownCellValue() const
	{
		return knownCellValue.value();
	}
	void addToKnownCellValue(int a)
	{
		knownCellValue.value()+=a;
	}
	void forgetCellValue()
	{
		knownCellValue=std::nullopt;
	}
	void rememberCellValue(int v)
	{
		knownCellValue=v;
	}
};
