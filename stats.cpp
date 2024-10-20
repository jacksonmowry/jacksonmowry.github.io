#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

using namespace std;

int main() {
  vector<double> inputs;
  double input;
  while (cin >> input) {
    inputs.push_back(input);
  }

  cout << "Size:   " << inputs.size() << endl;

  // Mean
  double total = 0;
  for (auto d : inputs) {
    total += d;
  }

  cout << "Mean:   " << total / inputs.size() << endl;

  // Median
  sort(inputs.begin(), inputs.end());
  size_t l = inputs.size() / 2 - 1;
  size_t r = inputs.size() - (inputs.size() / 2);

  cout << "Median: " << (inputs[l] + inputs[r]) / 2 << endl;
}
