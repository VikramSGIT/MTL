// This file need not be compiled with your project.
// This file is only intended for internal testing purposes.


#define PASS(X) std::cout << "\nTest " << X << " passed\n" << std::endl
#define FAIL(X) std::cout << "\nTest " << X << " failed\n" << std::endl
#define RUN(X) std::cout << "\nRunning Test " << X << "\n" << std::endl

#include "Memory/MemoryManager.h"
#include "String.h"
#include "Vector.h"

#include <iostream>
#include <sstream>

using namespace ME;

int main() {
	try {
		ME_MEM_INIT();
	}
	catch (...) {
		std::cout << "Memory initialization failed! Aborting..." << std::endl;
		return -1;
	}
	std::cout << "Memory initialization success" << std::endl;

	std::stringstream ss;
	{
		// Memory test cases
		{
			RUN(1);
		jump11:
			// Testing plain allocation, deallocation and reallocation
			try {
				size_t* i = alloc<size_t>(1);
				i[0] = 9;
				size_t* j = alloc<size_t>(1);
				j[0] = 8;
				dealloc(j);
				dealloc(i);
			}
			catch (...) {
				FAIL(1.1);
				goto jump121;
			}
			PASS(1.1);

		jump121:
			RUN("1.2.1");
			// Testing reallocation skipping
			try {
				int* i = alloc<int>(1);
				i[0] = 9;
				realloc(i, 1);
				i[1] = 8;
				dealloc(i);
			}
			catch (...) {
				FAIL(1.2);
				goto jump122;
			}
			PASS(1.2);
		jump122:
			RUN("1.2.2");
			//Testing reallocation expanding
			try {
				size_t* i = alloc<size_t>(1);
				i[0] = 7;
				realloc(i, 1);
				i[1] = 8;
				dealloc(i);
			}
			catch (...) {
				FAIL("1.2.1");
				goto jump13;
			}
			PASS("1.2.1");
		jump13:
			RUN(1.3);
			// Testing reallocation copying
			try {
				size_t* i = alloc<size_t>(1);
				i[0] = 9;
				int* j = alloc<int>(1);
				j[0] = 8;
				realloc(i, 1);
				i[1] = 7;
				dealloc(j);
				dealloc(i);
			}
			catch (...) {
				FAIL(1.3);
				goto jump141;
			}

		jump141:
#ifdef ME_ENABLE_OVERFLOW_CHECK
			{
				RUN("1.4.1");
				// Testing bufferoverflow check
				size_t* i = alloc<size_t>(1);
				try {
					i[0] = 9;
					i[1] = 8; // overwriting guradbytes
					dealloc(i);
				}
				catch (...) {
					forced_dealloc(i);
					PASS("1.4.1");
					goto jump142;
				}
				FAIL("1.4.1");

			jump142:
				RUN("1.4.2");
				i = alloc<size_t>(1);
				try {
					i[0] = 9;
					*(i - 1) = 8; // overwriting guardbytes
					dealloc(i);
				}
				catch (...) {
					forced_dealloc(i);
					PASS("1.4.2");
					goto jump15;
				}
				FAIL("1.4.2");
			}
#endif
		jump15:
			RUN(1.5);
			// Bulk allocation, reallocation and deallocation
			try {
				size_t* i = alloc<size_t>(10000);
				realloc(i, 20000);
				dealloc(i);
			}
			catch (...) {
				FAIL(1.5);
				goto jump21;
			}
			PASS(1.5);
		}
		jump21:
#ifdef ME_STRING
		//String test cases
		RUN(2);
		{
			// Plain test
			RUN(2.1);
			try {
				string s = "Hey";
				s += ',';
				s += " This is a test";


				string ss = '!';
				s += ss;
				std::cout << s.c_str() << std::endl;
			}
			catch (...) {
				FAIL(2.1);
				goto jump22;
			}
			PASS(2.1);

		jump22:
			// Assignment test
			RUN(2.2);
			try {
				string s;
				s = 'H';
				s = "Hey ";

				string ss = "Hey, This is a test!";
				s = ss;
				std::cout << s.c_str() << std::endl;
			}
			catch (...) {
				FAIL(2.2);
				goto jump23;
			}
			PASS(2.2);

		jump23:
			// Addition operator test
			RUN(2.3);
			try {
				string s = "Hey";
				s = s + ',';
				s = s + " This is a test";

				string ss = '!';
				s = s + ss;
				std::cout << s.c_str() << std::endl;
			}
			catch (...) {
				FAIL(2.3);
				goto jump24;
			}
			PASS(2.3);

		jump24:
			// Capcity exploitation test
			RUN(2.3);
			try {
				string s = "Hey, This is a test!";
				s = "Hey";
				s += ',';
				s += " This is a test";

				string ss = '!';
				s += ss;
				std::cout << s.c_str() << std::endl;
			}
			catch (...) {
				FAIL(2.4);
				goto jump25;
			}
			PASS(2.4);

		jump25:
			//Erase test
			RUN(2.4);
			try {
				string s = "Hey, This is a test!?";
				s.erase(s.begin() + 20);
				std::cout << s.c_str() << std::endl;
			}
			catch (...) {
				FAIL(2.5);
				goto jump26;
			}
			PASS(2.5);
		
		jump26:
			RUN(2.6);
			// clear and release
			try {
				string s = "Hey, This is a test!";
				s.clear();

				string ss = "Hey, This is a test!";
				s.release();
			}
			catch (...) {
				FAIL(2.6);
				goto jump3;
			}
			PASS(2.6);
		}
		
#endif
		jump3:
#ifdef ME_VECTOR
#define PRINTVEC(X) for(auto item : X) {std:: cout << item << " ";} std::cout << std::endl
		// Vector Test
		RUN(3);
		{
			// Initialization Test
			try {
				vector<size_t> i({ 1,2,3,4,5 });
				PRINTVEC(i);

				i = { 1,2,3,4,5 };
				PRINTVEC(i);

				vector<size_t> j({ 6,7,8,9,0 });
				i = j;
				PRINTVEC(i);
			}
			catch (...) {
				FAIL(3.1);
				goto jump32;
			}
			PASS(3.1);

		jump32:
			// push and emplace test
			try {
				vector<size_t> i({ 1,2,3,4 });
				i.push_back(6);
				PRINTVEC(i);

				i.emplace_back(8);
				PRINTVEC(i);

				i.push(&(i[4]), 5);
				PRINTVEC(i);

				i.push(&(i[6]), 7);
				PRINTVEC(i);

				i.emplace(i.end(), 9);
				PRINTVEC(i);

				i.emplace(i.begin(), 0);
				PRINTVEC(i);
			}
			catch (...) {
				FAIL(3.2);
				goto jump33;
			}
			PASS(3.2);


			//Testing Erase
		jump33:
			try {
				vector<int> i = { 1, 2, 3, 4, 5 };
				i.erase(&i[3]);
				PRINTVEC(i);
			}
			catch (...) {
				FAIL(3.3);
				goto jump34;
			}
			PASS(3.3);

		jump34:
			RUN(3.4);
			// Testing clear and release
			try {
				vector<int> i = { 1, 2, 3, 4, 5 };
				i.clear();

				vector<int> j = { 1, 2, 3, 4, 5 };
				j.release();
			}
			catch (...) {
				FAIL(3.4);
				goto jump4;
			}
			PASS(3.4);
		}

		jump4:
		// Test for CString vectors
		RUN(4);
		{
			// Initialization Test
			RUN(4.1);
			try {
				vector<const char*> i({ "Hey,", "This", "is", "a", "test!" });
				PRINTVEC(i);

				i = { "Hey,", "This", "is", "a", "test!" };
				PRINTVEC(i);

				vector<const char*> j = { "Hey,", "This", "is", "other", "test!" };
				i = j;
				PRINTVEC(i);
			}
			catch (...) {
				FAIL(4.1);
				goto jump42;
			}
			PASS(4.1);

		jump42:
			// push and emplace test
			RUN(4.2);
			try {
				vector<const char*> i;
				i.push_back("Hey,");
				PRINTVEC(i);
				i.emplace_back("is");
				PRINTVEC(i);

				i.push(i.begin() + 1, "This");
				PRINTVEC(i);
				i.emplace(i.end(), "a test!");
				PRINTVEC(i);
			}
			catch (...) {
				FAIL(4.2);
				goto jump43;
			}
			PASS(4.2);

		jump43:
			// erase test
			try {
				vector<const char*> i({ "Hey,", "This", "is", "a", "test!" });
				i.erase(i.begin());
				PRINTVEC(i);
			}
			catch (...) {
				FAIL(4.3);
				goto jump44;
			}
			PASS(4.3);

		jump44:
			// clear and release test
			try {
				vector<const char*> i({ "Hey,", "This", "is", "a", "test!" });
				i.clear();

				vector<const char*> j({ "Hey,", "This", "is", "a", "test!" });
				j.release();
			}
			catch (...) {
				FAIL(4.4);
				goto jump5;
			}
			PASS(4.4);
		}
#endif
	jump5:
		// Ref test
	jumpend:
		try {
			ME_MEM_CLEAR();
		}
		catch (...) {
			std::cout << "Error while exiting" << std::endl;
		}
		std::cout << "No Errors while exiting" << std::endl;
	}
}