// k5/lexicmp.cc
#include<algorithm>
#include<iostream>
#include<functional>

char text1[] = "Arthur";
int length1  = sizeof(text1);
char text2[] = "Vera";
int length2  = sizeof(text2);

using namespace std;

int main ()
{
  if(lexicographical_compare(
            text1, text1 + length1,
            text2, text2 + length2))
    cout << text1 << " comes before " << text2 << endl;
  else
    cout << text2 << " comes before " << text1 << endl;

  if(lexicographical_compare(
            text1, text1 + length1,
            text2, text2 + length2,
            greater<char>()))      // reverse sorting order
    cout << text1 << " comes after " << text2 << endl;
  else
    cout << text2 << " comes after " << text1 << endl;
}
