//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001 by holders identified in authors.txt
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//========================================================================================

#include "serbinfile.h"
#include "platform.h"

using namespace std;

void serBinFile::setVersion(unsigned int __version)
{
	_version = __version;
}

unsigned int serBinFile::getVersion()
{
	return _version;
}

unsigned int serBinFile::size()
{
	return _count;
}

void serBinFile::prepareReading(std::string ident)
{
	std::string fileName(ident);
	fileName.append(".bin");
	file.open(fileName.c_str(), ios::in | ios::binary);
	file.read((char*)&_version, 4);
	file.read((char*)&_count, 4);
	ISerialization::prepareReading(ident);
}

void serBinFile::prepareWritting(std::string ident)
{
	std::string fileName(ident);
	fileName.append(".bin");
	file.open(fileName.c_str(), ios::out | ios::binary);
	_count = 0;
	// First line in file is version
	file.write((char*)&_version, 4);
	file.write((char*)&_count, 4);
	ISerialization::prepareWritting(ident);
}

void serBinFile::close()
{
	if ( isWritting() )
	{
		file.seekp(4);
		file.write((char*)&_count, 4);
	}
	file.close();
}

void serBinFile::writeObjectID(string data)
{
	write("objectID", data);
	_count++;
}

void serBinFile::write(std::string Key, std::string &data)
{
	UI32 uiSize = data.size();
	file.write((char*)&uiSize, 4);
	if (uiSize != 0)
		file.write((char*)data.c_str(), uiSize);
}

void serBinFile::write(std::string Key, unsigned int data)
{
	file.write((char*)&data, 4);
}

void serBinFile::write(std::string Key, signed int data)
{
	file.write((char*)&data, 4);
}

void serBinFile::write(std::string Key, signed short data)
{
	file.write((char*)&data, 2);
}

void serBinFile::write(std::string Key, unsigned short data)
{
	file.write((char*)&data, 2);
}

void serBinFile::write(std::string Key, unsigned char data)
{
	file.write((char*)&data, 1);
}

void serBinFile::write(std::string Key, signed char data)
{
	file.write((char*)&data, 1);
}

void serBinFile::write(std::string Key, bool data)
{
	file.write((char*)&data, 1);
}

void serBinFile::doneWritting()
{

}

void serBinFile::readObjectID(string &data)
{
	read("objectID", data);
}

void serBinFile::read(std::string Key, string& data)
{
	UI32 uiSize = 0;
	file.read((char*)&uiSize, 4);
	if (!file.fail())
	{
		if ( uiSize != 0 )
		{
			char* c_Str = new char[uiSize + 1];
			file.read(c_Str, uiSize);
			c_Str[uiSize] = 0;
			data = c_Str;
			delete [] c_Str;
		}
	}
	else
		data = "";
}

void serBinFile::read(std::string Key, unsigned int  &data)
{
	file.read((char*)&data, 4);
}

void serBinFile::read(std::string Key, signed   int  &data)
{
	file.read((char*)&data, 4);
}

void serBinFile::read(std::string Key, signed short &data)
{
	file.read((char*)&data, 2);
}

void serBinFile::read(std::string Key, unsigned short &data)
{
	file.read((char*)&data, 2);
}

void serBinFile::read(std::string Key, unsigned char &data)
{
	file.read((char*)&data, 1);
}

void serBinFile::read(std::string Key, signed   char &data)
{
	file.read((char*)&data, 1);
}

void serBinFile::read(std::string Key, bool &data)
{
	file.read((char*)&data, 1);
}
