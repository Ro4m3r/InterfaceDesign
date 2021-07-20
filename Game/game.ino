#include <Adafruit_NeoPixel.h>
#include <CapacitiveSensor.h>
#include <Keyboard.h>

///--- Constants ---///
const uint8_t NEOPIXEL_DATA_PIN = 0;
const uint8_t SENSOR_COUNT = 4;
const uint8_t CAPACITIVE_WRITE_PIN = 1;
const uint8_t CAPACITANCE_SAMPLE_COUNT = 30;
const uint16_t CAPACITANCE_THRESHOLD = 2000;
const uint8_t PIXEL_BRIGHTNESS = 50;
const uint8_t CORRECT_FOR_DIFFICULTY_INCREASE = 2;
const uint16_t STARTING_SEQUENCE_TIME = 750;
const uint8_t INITIAL_SEQUENCE_LENGTH = 4;
const uint8_t SEQUENCE_INCREASE_PER_DIFFICULTY = 1;
const float SEQUENCE_TIME_DECAY = 0.7f;

///--- NeoPixels ---///
Adafruit_NeoPixel pixels(SENSOR_COUNT, NEOPIXEL_DATA_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel floraPixel(1, 8, NEO_GRB + NEO_KHZ800);

///--- Colors ---///
uint32_t colors[SENSOR_COUNT] = 
{
    pixels.Color(255, 0, 0),
    pixels.Color(0, 255, 0),
    pixels.Color(0, 0, 255),
    pixels.Color(255, 0, 255)
};

uint32_t waitColor = pixels.Color(255, 0, 0);
uint32_t playColor = pixels.Color(0, 255, 0);

///--- Sensors ---///
CapacitiveSensor sensors[SENSOR_COUNT] = 
{
    CapacitiveSensor(CAPACITIVE_WRITE_PIN, 12),
    CapacitiveSensor(CAPACITIVE_WRITE_PIN, 6),
    CapacitiveSensor(CAPACITIVE_WRITE_PIN, 9),
    CapacitiveSensor(CAPACITIVE_WRITE_PIN, 10)
};

///--- Keys ---///
const bool USE_KEYS = true;

char keys[SENSOR_COUNT] = 
{
    'a',
    's',
    'd',
    'f'
};

///--- Global Variables ---///
bool gameIsRunning = false;
bool isPlayerTurn = false;
uint8_t currentDifficulty = 0;
uint8_t currentSequenceIndex = 0;
uint8_t* currentSequence = NULL;
uint8_t currentSequenceLength = 0;

void setup()
{
    randomSeed(0);
    floraPixel.begin();
    clearFloraPixel();
    floraPixel.setBrightness(PIXEL_BRIGHTNESS);
    floraPixel.show();
    pixels.begin();
    clearAllPixels(false);
    pixels.setBrightness(PIXEL_BRIGHTNESS);
    pixels.show();
    Keyboard.begin();
}

void loop()
{
    handleGameStart();
    handleGameLoop();
    delay(10);
}

void handleGameLoop()
{
    if (!gameIsRunning) return;

    if (!isPlayerTurn)
    {
        setFloraPixel(waitColor);
        clearAllPixels(true);
        delay(500);

        currentSequenceLength = (currentDifficulty / CORRECT_FOR_DIFFICULTY_INCREASE) * SEQUENCE_INCREASE_PER_DIFFICULTY + INITIAL_SEQUENCE_LENGTH;
        uint16_t playInterval = pow(SEQUENCE_TIME_DECAY, currentDifficulty / CORRECT_FOR_DIFFICULTY_INCREASE) * STARTING_SEQUENCE_TIME;

        currentSequence = generateSequence(currentSequenceLength);

        playSequence(currentSequence, currentSequenceLength, playInterval);

        clearAllPixels(true);

        currentSequenceIndex = 0;

        isPlayerTurn = true;

        setFloraPixel(playColor);
    }
    else
    {
        for (int i = 0; i < SENSOR_COUNT; i++)
            if (!handleButton(i))
                break;

        pixels.show();
    }
}

uint8_t handleButton(uint8_t id)
{
    uint64_t capacitance = sensors[id].capacitiveSensor(CAPACITANCE_SAMPLE_COUNT);

    if (!isPressingButton(id)) return true;

    activatePixel(id, true);
    if (USE_KEYS) Keyboard.press(keys[id]);

    while (isPressingButton(id))
    {
        delay(10);
    }

    clearAllPixels(true);
    if (USE_KEYS) Keyboard.release(keys[id]);

    if (currentSequence[currentSequenceIndex] == id)
    {
        currentSequenceIndex++;

        if (currentSequenceIndex == currentSequenceLength)
        {
            isPlayerTurn = false;
            delete[] currentSequence;
            currentDifficulty++;
            flashFloraPixel(3, playColor, 200);
        }

        return false;
    }
    else
    {
        flashFloraPixel(3, waitColor, 200);

        resetGame();

        return false;
    }
}

void resetGame()
{
    gameIsRunning = false;
}

void handleGameStart()
{
    if (gameIsRunning) return;

    clearFloraPixel();
    clearAllPixels(true);

    bool isPressingAllButtons = true;

    for (int i = 0; i < SENSOR_COUNT; i++)
        if (!isPressingButton(i))
            isPressingAllButtons = false;

    if (isPressingAllButtons)
    {
        for (int i = 0; i < SENSOR_COUNT; i++)
        {
            activatePixel(i, true);
            delay(100);
        }

        gameIsRunning = true;
        isPlayerTurn = false;
        currentDifficulty = 0;
        currentSequenceIndex = 0;
    }
}

bool isPressingButton(uint8_t id)
{
    return sensors[id].capacitiveSensor(CAPACITANCE_SAMPLE_COUNT) > CAPACITANCE_THRESHOLD;
}

void playSequence(uint8_t* sequence, uint8_t length, uint16_t interval)
{
    for (uint8_t i = 0; i < length; i++)
    {
        uint8_t id = sequence[i];

        clearAllPixels(false);

        activatePixel(id, false);
        if (USE_KEYS) Keyboard.press(keys[id]);

        pixels.show();

        delay(interval / 2);

        clearAllPixels(true);
        if (USE_KEYS) Keyboard.release(keys[id]);

        delay(interval / 2);
    }
}

uint8_t* generateSequence(uint8_t length)
{
    uint8_t* sequence = new uint8_t[length];

    for (uint8_t i = 0; i < length; i++)
    {
        sequence[i] = (uint8_t)random(0, SENSOR_COUNT);
    }

    return sequence;
}

void clearAllPixels(bool show)
{
    for (uint8_t i = 0; i < SENSOR_COUNT; i++)
        pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    if (show) pixels.show();
}

void activatePixel(uint8_t id, bool show)
{
    pixels.setPixelColor(id, colors[id]);
    if (show) pixels.show();
}

void setFloraPixel(uint32_t color)
{
    floraPixel.setPixelColor(0, color);
    floraPixel.show();
}

void clearFloraPixel()
{
    floraPixel.setPixelColor(0, floraPixel.Color(0, 0, 0));
    floraPixel.show();
}

void flashFloraPixel(uint8_t count, uint32_t color, uint16_t interval)
{
    for (uint8_t i = 0; i < count; i++)
    {
        setFloraPixel(color);
        delay(interval / 2);
        clearFloraPixel();
        delay(interval / 2);
    }
}