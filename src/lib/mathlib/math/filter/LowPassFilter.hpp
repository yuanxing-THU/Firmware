
#pragma once

namespace math
{

template <typename T>
class __EXPORT LowPassFilter
{
public:
	/**
	 * Constructor with filtering disabled
	 */
	LowPassFilter() : _rc(0.0), _time_last(0)
	{
	}

	/**
	 * Constructor with cutoff frequency
	 */
	explicit LowPassFilter(float cutoff_freq) : _time_last(0)
	{
		set_cutoff_frequency(cutoff_freq);
	}

	/**
	 * Change filter parameters
	 */
	void set_cutoff_frequency(float cutoff_freq) {
		if (cutoff_freq <= 0.0f)
			_rc = 0.0;
		else
			_rc = 1.0 / M_TWOPI / (double) cutoff_freq;
	}

	/**
	 * Return the cutoff frequency
	 */
	float get_cutoff_freq(void) const {
		if (_rc > 0.0)
			return (float) (1.0 / M_TWOPI / _rc);
		else
			return 0.0f;
	}

	/**
	 * Add a new value to the filter
	 *
	 * @return retrieve the filtered result
	 */
	const T &apply(uint64_t t, const T &next_value) {
		if (_rc > 0.0) {
			double dt = (t - _time_last) * 1.0e-6;
			double a = dt / (_rc + dt);
			_value *= (T) (1.0 - a);
			_value += next_value * (T) a;
			if (_time_last == 0) {
				_value = next_value; // init first sample
			}
			_time_last = t;
			return _value;
		}
		else {
			return next_value;
		}
	}

	/**
	 * Reset the filter state to this value
	 */
	void reset(uint64_t time, T value) {
		_value = value;
		_time_last = time;
	}

private:
	double _rc;
	uint64_t _time_last;
	T _value;
};

} // namespace math
