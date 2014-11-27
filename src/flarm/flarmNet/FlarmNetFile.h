#ifndef FLARMNETFILE_H_
#define FLARMNETFILE_H_

#include <QByteArray>
#include <QString>
#include <QStringList>

#include "src/flarm/flarmNet/FlarmNetRecord.h"

/**
 * Some helper methods for handling the FlarmNet file format
 *
 * Data representations:
 *   - plain line
 *     A QString containing one line of plain text
 *     Example: "FFF0011FSV-Gerstetten..."
 *     Multiple plain lines are represented as a QStringList
 *   - encoded line
 *     A QString computed from the plain data by encoding it as latin1,
 *     converting each byte to a two-digital hexadecimal representation and
 *     concatenating them.
 *     Example: "4646463031314653562d4765727374657474656e..."
 *   - encoded file
 *     A QString containing multiple encoded lines, concatenated with LF ('\n');
 *     additionally, a header line is prepend.
 *     Example: "00136b\n464646303131...\n464646464646..."
 *   - raw file: a QByteArray containing an encoded file, encoded as ASCII or
 *     any encoding compatible with ASCII.
 *     Example: { 0x34, 0x36, 0x34, 0x36, 0x34, 0x36, 0x33, 0x30, ..., 0x0A, ...}
 *     Note that only digits, letters from 'a' to 'f' and LF can appear in the
 *     encoded file.
 *
 * The terms "line" and "file" without a qualifier denote an encoded line or
 * file, respectively.
 */
class FlarmNetFile
{
	public:
		static QString     decodeLine    (const QString     &encodedLine );
		static QStringList decodeLines   (const QStringList &encodedLines);
		static QStringList decodeFile    (const QString     &encodedFile );
		static QStringList decodeRawFile (const QByteArray  &rawFile     );

		static QString     encodeLine    (const QString     &plainLine );
		static QStringList encodeLines   (const QStringList &plainLines);
		static QString     encodeFile    (const QStringList &plainLines);
		static QByteArray  encodeFileRaw (const QStringList &plainLines);

		static FlarmNetRecord createRecord (const QString &plainLine, bool *ok);
		static QList<FlarmNetRecord> createRecords (const QByteArray &rawFile, int *numGood, int *numBad);
};

#endif
