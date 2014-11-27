//#ifndef TEMPDIR_H_
//#define TEMPDIR_H_
//
//#include <cstring>
//#include <QString>
//
//#include "src/accessor.h"
//#include "src/SkException.h"
//
//class TempDir
//{
//	public:
//		class ex_create_error: public SkException
//		{
//			public:
//				ex_create_error (int _errno) { errno=_errno; desc=strerror (errno); }
//				~ex_create_error () throw () {};
//				QString description () const { return desc; }
//
//				int errno;
//				QString desc;
//		};
//
//		TempDir (const QString &id="temp_dir.") throw (ex_create_error);
//		~TempDir ();
//
//		RO_ACCESSOR (QString, name)
//
//	private:
//		QString name;
//};
//
//#endif

