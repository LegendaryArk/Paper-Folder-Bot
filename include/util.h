class Util
{
public:
	static inline double clamp(double value, double min, double max)
	{
		if (value < min) return min;
		if (value > max) return max;
		return value;
	}

	static inline int sgn(double value)
	{
		return (value > 0) - (value < 0);
	}
};