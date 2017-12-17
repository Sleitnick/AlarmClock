/*
 * Author: Stephen Leitnick
 * Date: November 2017
 *
 * AlarmSoundFragment simply stores a tone
 * value and a tone duration. It essentially
 * says "play this tone value for this long"
 */

class AlarmSoundFragment {
	public:
		const int value;
		const int duration;
		AlarmSoundFragment(int toneValue, int toneDuration);
};

