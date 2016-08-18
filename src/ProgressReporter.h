#pragma once
#include <iostream>
#include <QString>

class ProgressReporter
{
public:
	//Used to report progress, double should be value from 0 to 1.0, QString should be info message
	virtual void operator()(double, QString) = 0;
	
	virtual void set_min(double val) { min_value = val; }
	virtual void set_max(double val) { max_value = val; }

	virtual	double get_min(double val) { return min_value; }
	virtual double get_max(double val) { return max_value; }
	virtual double get_val(double val) { return val; }

protected:
    double val;
    double min_value;
    double max_value;
};

class NoProgressReporter : public ProgressReporter
{
	void operator()(double, QString) {} 
};

class ConsoleProgressReporter : public ProgressReporter
{
public:
	ConsoleProgressReporter(double min, double max) { min_value = min; max_value = max; }

	void operator()(double value, QString message) {
		std::cout << ((value - (double)min_value) / ((double)max_value - (double)min_value)) * 100.0 << "%: " << message.toStdString();
	}
};