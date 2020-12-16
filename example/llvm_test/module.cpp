#include <random>

static double getMagicNumber()
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_real_distribution<double> dist(1.0, 10.0);
  return dist(mt);
}

extern "C" double magic()
{
  return getMagicNumber();
}