//#include "TempDir.h"
//
//#include <iostream>
//
//#include "errno.h"
//#include "malloc.h"
//#include "stdlib.h"
//
//#include "src/util/qString.h"
//
//TempDir::TempDir (const QString &id)
//	throw (ex_create_error)
//	// Name will be /tmp/${id}XXXXXX
//{
//	QString name_template=id;
//	QString::const_iterator end=name_template.end ();
//	for (QString::iterator it=name_template.begin (); it!=end; ++it)
//	{
//		if (QString::fromAscii ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_-.").find (*it)<0)
//		{
//			(*it)='_';
//		}
//	}
//
//	name_template="/tmp/"+name_template+"XXXXXX";
//
//	// Need to copy to char * manually
//	int len=name_template.size ();
//	char *dirname_buffer=new char[len+1];
//	strcpy (dirname_buffer, name_template.utf8 ().constData ());
//
//	// Make the temporary directory
//	char *dirname_ret=mkdtemp (dirname_buffer);
//
//	if (dirname_ret)
//	{
//		name=dirname_ret;
//		delete[] dirname_buffer;
//	}
//	else
//	{
//		delete[] dirname_buffer;
//		// EINAVL: name does not end with XXXXXX
//		throw ex_create_error (errno);
//	}
//}
//
//TempDir::~TempDir ()
//{
//	if (!name.isEmpty ())
//	{
//		if (name.left (5)!="/tmp/")
//		{
//			// For safety reasons, do not delete this.
//			std::cerr << "Error: the directory name \""+name+"\" does not start with /tmp/.";
//		}
//		else if (name.find ("/../")>=0)
//		{
//			std::cerr << "Error: the directory name \""+name+"\" contains /../.";
//		}
//		else
//		{
//			system ((qnotr ("rm -r ")+name).utf8 ().constData ());
//			//system ((qnotr ("touch ")+name+qnotr (".delete")).c_str ());
//		}
//	}
//}

