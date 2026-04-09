#include <bits/stdc++.h>
using namespace std;

bool detect_overlap(const vector<int> &arr1, const vector<int> &arr2) {
  if (arr1[0] <= arr2[1] && arr2[0] <= arr1[1]) {
    return true;
  }
  return false;
}

bool can_resolve(const vector<vector<int>> &store, int skip_idx, int cakes) {
  int space_needed = store[skip_idx][1] - store[skip_idx][0] + 1;

  vector<vector<int>> temp;
  for (int i = 0; i < store.size(); i++) {
    if (i != skip_idx) {
      temp.push_back(store[i]);
    }
  }

  for (int i = 0; i < (int)temp.size() - 1; i++) {
    if (detect_overlap(temp[i], temp[i + 1]))
      return false;
  }

  if (temp.empty())
    return true;

  if (temp[0][0] - 1 >= space_needed)
    return true;

  if (cakes - temp.back()[1] >= space_needed)
    return true;

  for (int i = 0; i < (int)temp.size() - 1; i++) {
    if (temp[i + 1][0] - temp[i][1] - 1 >= space_needed)
      return true;
  }

  return false;
}

int main() {
  int t;
  if (cin >> t) {
    while (t--) {
      int cake_num, child_num, k;
      cin >> cake_num >> child_num >> k;

      vector<vector<int>> store(child_num, vector<int>(2));
      for (int i = 0; i < child_num; i++) {
        cin >> store[i][0] >> store[i][1];
      }

      sort(store.begin(), store.end());

      if (k == 0) {
        bool conflict = false;

        for (int i = 0; i < child_num - 1; i++) {
          if (detect_overlap(store[i], store[i + 1])) {
            conflict = true;
            break;
          }
        }

        if (conflict)
          cout << "Bad\n";
        else
          cout << "Good\n";
      }

      if (k == 1) {
        bool conflict = false;

        for (int i = 0; i < child_num - 1; i++) {
          if (detect_overlap(store[i], store[i + 1])) {

            bool resolved = can_resolve(store, i, cake_num) ||
                            can_resolve(store, i + 1, cake_num);

            if (resolved) {
              conflict = false;
            } else {
              conflict = true;
            }
            break;
          }
        }

        if (conflict)
          cout << "Bad\n";
        else
          cout << "Good\n";
      }
    }
  }
  return 0;
}
