#define _CRT_SECURE_NO_WARNINGS 1
#pragma once
#include<iostream>
#include<assert.h>
using namespace std;

#include"Allocate.h"
#include"Construct.h"
#include"Uninitialized.h"
#include "Allocate.h"
#include"Vector.h"

#include"List.h"

int main()
{
	//TestVector();

	//TestAllocate1();
	//TestAllocate2();
	//TestAllocate3();

	TestList();

	system("pause");
	return 0;
}