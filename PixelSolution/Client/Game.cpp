#include "Game.h"
#include <string>
#include <iostream>
#include "PixelEngineAPI.h"
void Start()
{
	PObject* Object			= CreateObject("GameObject");
	std::string TypeName	= GetObjectTypeName(Object);
	std::string ParentName	= GetObjectParentName(Object);
	std::string ChildName	= GetObjectChildName(Object);
	std::cout << "TypeName :"  + TypeName<< std::endl;
	std::cout << "ParentName :"+ ParentName<< std::endl;
	std::cout << "ChildName :" + ChildName<< std::endl;

	int FieldMaxCount  = GetObjectFieldMaxCount(Object);
	int MethodMaxCount = GetObjectMethodMaxCount(Object);

	std::cout << "FieldMaxCount :" + std::to_string(FieldMaxCount)<< std::endl;
	std::cout << "MethodMaxCount :" + std::to_string(MethodMaxCount)<< std::endl;

	for (int i = 0; i < FieldMaxCount; i++)
	{
		std::string FieldName = GetObjectFieldName(Object,i);
		std::string FieldType = GetObjectFieldType(Object,i);

		std::cout << "FieldName :" + FieldName<< std::endl;
		std::cout << "FieldType :" + FieldType<< std::endl;
	}
}

void Update()
{

}
