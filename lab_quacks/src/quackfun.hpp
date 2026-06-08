/**
 * @file quackfun.cpp
 * This is where you will implement the required functions for the
 * stacks and queues portion of the lab.
 */
#include <queue>
#include <stack>
using namespace std;

namespace QuackFun {

/**
 * Sums items in a stack.
 *
 * **Hint**: think recursively!
 *
 * @note You may modify the stack as long as you restore it to its original
 * values.
 *
 * @note You may use only two local variables of type T in your function.
 * Note that this function is templatized on the stack's type, so stacks of
 * objects overloading the + operator can be summed.
 *
 * @note We are using the Standard Template Library (STL) stack in this
 * problem. Its pop function works a bit differently from the stack we
 * built. Try searching for "stl stack" to learn how to use it.
 *
 * @param s A stack holding values to sum.
 * @return  The sum of all the elements in the stack, leaving the original
 *          stack in the same state (unchanged).
 */
template <typename T> T sum(stack<T> &s) {
  if (s.empty())
    return 0;

  T curr = s.top();
  s.pop();

  T summ = curr + sum(s);
  s.push(curr);
  // Your code here
  return summ; // stub return value (0 for primitive types). Change this!
               // Note: T() is the default value for objects, and 0 for
               // primitive types
}

/**
 * Checks whether the given string (stored in a queue) has balanced brackets.
 * A string will consist of parentheses, curly/square brackets and other
 * characters. This function will return true if and only if the bracket
 * characters in the given string are balanced. For this to be true, all
 * brackets must be matched up correctly, with no extra, hanging, or unmatched
 * brackets. For example, the string "[hello=](){}" is balanced, "{()[{a}]}" is
 * balanced, "[})" is unbalanced, "][" is unbalanced, and "))))[cs225]" is
 * unbalanced.
 *
 * For this function, you may only create a single local variable of type
 * `stack<char>`! No other stack or queue local objects may be declared. Note
 * that you may still declare and use other local variables of primitive types.
 *
 * @param input The queue representation of a string to check for balanced
 * brackets in
 * @return      Whether the input string had balanced brackets
 */
bool isBalanced(queue<char> input) {
  stack<char> q;
  while (!input.empty()) {
    char curr = input.front();
    input.pop();
    switch (curr) {
    case '[':
      q.push(curr);
      break;
    case '{':
      q.push(curr);
      break;
    case '(':
      q.push(curr);
      break;
    case ']':
      if (q.empty() || q.top() != '[')
        return false;
      q.pop();
      break;
    case ')':
      if (q.empty() || q.top() != '(')
        return false;
      q.pop();
      break;
    case '}':
      if (q.empty() || q.top() != '{')
        return false;
      q.pop();
      break;
    }
  }
  if (q.empty())
    return true;
  return false;
}

/**
 * Reverses even sized blocks of items in the queue. Blocks start at size
 * one and increase for each subsequent block.
 *
 * **Hint**: You'll want to make a local stack variable.
 *
 * @note Any "leftover" numbers should be handled as if their block was
 * complete.
 *
 * @note We are using the Standard Template Library (STL) queue in this
 * problem. Its pop function works a bit differently from the stack we
 * built. Try searching for "stl stack" to learn how to use it.
 *
 * @param q A queue of items to be scrambled
 */
template <typename T> void scramble(queue<T> &q) {
  queue<T> q2;

  int should_reverse = 1;
  stack<T> s;
  while (!q.empty()) {
    int iter = (should_reverse > (int)q.size()) ? (int)q.size() : should_reverse;
    if (should_reverse % 2 == 0) {
      for (int i = 0; i < iter; ++i) {
        T curr = q.front();
        q.pop();
        s.push(curr);
      }
      for (int i = 0; i < iter; ++i) {
        T curr = s.top();
        s.pop();
        q2.push(curr);
      }
    } else {
      for (int i = 0; i < iter; ++i) {
        T curr = q.front();
        q.pop();
        q2.push(curr);
      }
    }
    ++should_reverse;
  }
  while (!q2.empty()) {
    q.push(q2.front());
    q2.pop();
  }

  // Your code here
}
} // namespace QuackFun
