#include <iostream>
#include "inc/make_array.h"
#include <algorithm>
#include <string>
#include <iterator>

#include "inc/array_view.h"
#include "learning_ds.h"

using namespace std;

bool test_array_view() {
	char arr[] = { 1,2,4,5,6 };
	experiment::ArrayView<char> view{ arr };
	return true;
}

bool test_make_array() {
	auto const v = experiment::make_array(string{ "anil" },
		string{ "kumar" }, string{ "singh" });
	copy(v.begin(), v.end(), ostream_iterator<string>(cout, ","));
	return true;
}

int main() {
	//cout << "hello\n";
	test_make_array();
	test_array_view();
	static_assert(learning::enable_if<)
	return 0;
}