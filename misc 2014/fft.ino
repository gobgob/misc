IntervalTimer timer;

#define DEBUG_PIN_GENERAL 13
#define DEBUG_PIN_ON digitalWrite(DEBUG_PIN_GENERAL,HIGH)
#define DEBUG_PIN_OFF digitalWrite(DEBUG_PIN_GENERAL,LOW)

// Sample Size is > 2 * fe / F (because Shannon)
// fe = 62.5khZ and F = 1464
#define SAMPLE_SIZE 128

// Compute K = F * N / fe
#define FFT_K 3

#define FFT_VALUE_CMP 200

typedef struct FFTRingBuffer {
	uint16_t buffer[SAMPLE_SIZE];
	uint8_t head;
} FFTRingBuffer;

double sin_table[SAMPLE_SIZE];
double cos_table[SAMPLE_SIZE];

volatile static FFTRingBuffer FFTbuffer;

static void getSample();

inline void FFT_ring_buffer_init()
{
	memset((void *)&FFTbuffer,0,sizeof(FFTbuffer));
}

inline void FFT_ring_buffer_write(uint16_t value)
{
	FFTbuffer.buffer[FFTbuffer.head] = value;
	FFTbuffer.head = (FFTbuffer. head + 1 ) % SAMPLE_SIZE;
}

inline uint16_t FFT_ring_buffer_read(uint8_t value)
{
	return FFTbuffer.buffer[(FFTbuffer.head+value) % SAMPLE_SIZE];
}

double fft_compute_fft()
{
	// First, commit value from Ring buffer
	uint16_t buffer[SAMPLE_SIZE];
	uint8_t i;
	cli();
	for (i = 0; i < SAMPLE_SIZE; ++i)
	{
		buffer[i] = FFTbuffer.buffer[(FFTbuffer.head+i) % SAMPLE_SIZE];
	}
	sei();

	double real = 0, img = 0;
	for (i = 0; i < SAMPLE_SIZE; ++i)
	{
		real += buffer[i] * cos_table[i];
		img += buffer[i] * sin_table[i];
	}
	return sqrt(real * real + img * img);
}

void setup()
{
	pinMode(DEBUG_PIN_GENERAL,OUTPUT);
	Serial.begin(115200);

	uint8_t i;
	for (i = 0; i < SAMPLE_SIZE; ++i)
	{
		sin_table[i]=sin(-2*3.141592654*FFT_K*i/SAMPLE_SIZE);
		cos_table[i]=cos(-2*3.141592654*FFT_K*i/SAMPLE_SIZE);
	}

	FFT_ring_buffer_init();
	timer.begin(getSample, 1000000/62500);
}

void loop()
{
	DEBUG_PIN_ON;
	if (fft_compute_fft() >= FFT_VALUE_CMP) {
		digitalWrite(11,HIGH);
	} else {
		digitalWrite(11,LOW);
	}
	DEBUG_PIN_OFF;
	delay(1);
}


static void getSample()
{
	FFT_ring_buffer_write(analogRead(A0));
}
