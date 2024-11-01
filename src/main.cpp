#include <thread>
#include <mutex>
#include <condition_variable>
#include <SFML/Graphics.hpp>
#include <random>
#include <queue>
#include <chrono>
#include <cctype>
#include <iostream>

struct CharacterRenderData
{
  char character;
  sf::Vector2f position;
  sf::Color color;
};

std::queue<unsigned int> dataQueue;
std::queue<CharacterRenderData> renderQueue;
std::mutex dataQueueMutex;
std::mutex renderQueueMutex;
std::condition_variable cv;

const int X_MAX = 1000;
const int Y_MAX = 1000;
const int FONT_SIZE = 12;
const int SIZE = 2500;
bool MESSAGE_SHOWN = false;
bool PRODUCER_FINISHED = false;
bool CONSUMER_FINISHED = false;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<unsigned int> colorDist(0, 0xFFFFFF); // 24-bit unsigned int
std::uniform_int_distribution<int> sleepDist(0, 100);
std::uniform_int_distribution<int> charDist(33, 126);
std::uniform_int_distribution<int> xyDist(1, X_MAX - FONT_SIZE);

sf::Color convertToColor(unsigned int number)
{
  unsigned char r = (number >> 16) & 0xFF;
  unsigned char g = (number >> 8) & 0xFF;
  unsigned char b = number & 0xFF;
  return sf::Color(r, g, b);
}

char getCharacter()
{
  char randomChar = static_cast<char>(charDist(gen));
  return randomChar;
}

sf::Vector2f getPosition()
{
  sf::Vector2f pos = sf::Vector2f(xyDist(gen), xyDist(gen));
  return pos;
}

void producer()
{
  std::cout << "Producer thread has started..." << std::endl;
  for (int p=0; p<SIZE; ++p)
  {
    // generate number
    unsigned int color = colorDist(gen);
    // push random color onto dataQueue
    std::unique_lock<std::mutex> dataQueueLock(dataQueueMutex);
    dataQueue.push(color);
    //sleep random interval
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepDist(gen)));
    //alert consumer thread
    cv.notify_one();
  }
  PRODUCER_FINISHED = true;
}

void consumer()
{
  std::cout << "Consumer thread has started..." << std::endl;
  for (int c=0; c<SIZE; ++c)
  { 
    std::unique_lock<std::mutex> dataQueueLock(dataQueueMutex);
    // wait for data
    cv.wait(dataQueueLock, [] { return !dataQueue.empty(); });
    //retrieve number
    unsigned int number = dataQueue.front();
    dataQueue.pop();
    dataQueueLock.unlock();
    // set character, color, position
    CharacterRenderData charData;
    charData.color = convertToColor(number);
    charData.position = getPosition();
    charData.character = getCharacter();
    std::unique_lock<std::mutex> renderQueueLock(renderQueueMutex);
    renderQueue.push(charData);
    }
    CONSUMER_FINISHED = true;
}


int main()
{
  std::cout << "Creating window..." << std::endl;
  sf::RenderWindow window(sf::VideoMode(X_MAX, Y_MAX), "Multi-Threaded Art");

  std::cout << "Loading font..." << std::endl;
  sf::Font font;
  if(!font.loadFromFile("resources/FiraMono/FiraMonoNerdFontMono-Regular.otf"))
  {
    std::cout << "ERROR LOADING FONT!" << std::endl;
    return 1;
  }

  std::thread producerThread(producer);
  std::thread consumerThread(consumer);

  std::cout << "Creating texture..." << std::endl;
  sf::RenderTexture renderTexture;
  if(!renderTexture.create(X_MAX, Y_MAX))
  {
    std::cout << "ERROR CREATING RENDER TEXTURE!" << std::endl;
    return 1;
  }
  
  window.clear();
  std::cout << "Cleared window..." << std::endl;
  renderTexture.clear();
  std::cout << "Cleared texture..." << std::endl;

  std::cout << "Generating and rendering characters..." << std::endl;
  while (window.isOpen())
  {
    sf::Event event;
    while (window.pollEvent(event))
    {
      if (event.type == sf::Event::Closed)
        window.close();
    }

    while (!renderQueue.empty())
    {
      std::unique_lock<std::mutex> renderQueueLock(renderQueueMutex);
      CharacterRenderData& charData = renderQueue.front();
      sf::Text text;
      text.setFont(font);
      text.setString(charData.character);
      text.setPosition(charData.position);
      text.setFillColor(charData.color);
      text.setCharacterSize(FONT_SIZE);
      renderTexture.draw(text);
      renderQueue.pop();
      renderQueueLock.unlock();
      renderTexture.display();
    }
    renderTexture.display();
    sf::Sprite sprite(renderTexture.getTexture());
    window.draw(sprite);
    window.display();
    if (!MESSAGE_SHOWN && PRODUCER_FINISHED && CONSUMER_FINISHED)
    {
      std::cout << "Finished. When you close the window, your PNG file will be saved." << std::endl;
      MESSAGE_SHOWN = true;
    }
  }

  sf::Texture texture = renderTexture.getTexture();
  sf::Image screenshot = texture.copyToImage();
  if(!screenshot.saveToFile("multi_threaded_art.png"))
  {
    std::cout << "ERROR SAVING PNG!" << std::endl;
    return 1;
  }
  std::cout << "'multi_threaded_art.png' saved to project directory." << std::endl;

  std::cout << "Joining threads..." << std::endl;
  producerThread.join();
  consumerThread.join();
  std::cout << "Successfully joined threads." << std::endl;

  return 0;
}