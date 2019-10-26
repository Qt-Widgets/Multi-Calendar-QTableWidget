#ifndef PREANNOSTRUCTURE_H
#define PREANNOSTRUCTURE_H

#include <QtGui>
#include <QtWidgets>
#include <QApplication>
#include <QTableWidgetItem>


class CalendarItem : public QTableWidgetItem
{
public:
    CalendarItem( const QDate d );
    CalendarItem(const QString &text);
private:
    QDate day;
};


//
class DayCell
{
//
public:
 enum { MAGICNUMBER = 0x48611928, VERSION = 5 };
 DayCell();
 QIcon createColorToolButtonIcon( int setmodus );
 DayCell operator=( const DayCell d );
 /*bool DayCell::operator==( const DayCell &d ) const;*/
 void Register( int tr , int td , const QDate d ,  int gyear , int mod = 0 );
 CalendarItem *on_Table();
 bool isValid();
 inline operator QVariant() const {
    return QVariant::fromValue(*this);
  }
 inline QSize sizeHint() const  {
   return QSize(12,12);
 }
 QPixmap NoteIcon();
 int fromUday();   /* ID to save not free day  summ of day from 1.1.1970 to date */
 QColor BG( int setmodus );  /* background color from modus */
 QColor FC( int setmodus ); /* foreground from modus */
 QString text(); /* day as string */
 int day();  /* day nummer */
 int year(); 
 
  /* vars permanent */
    int td_line;  /* on model go to cell / cools  x*/
    int tr_line;  /* on model go to row */
    int modus; /* reservation modus    0= libero / 1= prenotato / 2= prenotato provvisorio / 3= riservato / 10 = chiuso / */
    int anno;
    QDate dax;
    QByteArray data;  /* reserve! image or so ... */
    QString note; 
 
};

Q_DECLARE_METATYPE(DayCell);

inline QDebug operator<<(QDebug debug, const DayCell& tline)
{
	debug.nospace() << "DayCell()" 
		    << tline.td_line << "," 
        << tline.tr_line << "," 
        << tline.modus << ",date()" 
        << tline.dax << "," 
        << tline.anno << ","
        << tline.data << ","    
        << tline.note << ")";
	return debug.space();
}

QDataStream& operator>>(QDataStream& in, DayCell& udoc);
QDataStream& operator<<(QDataStream& out, const DayCell& udoc);



/*

  /////  hard work ! 
class DateDelegate : public QItemDelegate
 {
     Q_OBJECT

 public:
     DateDelegate(QWidget *parent = 0) : QItemDelegate(parent) {}
     void drawBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
     void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
     QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
 private slots:
 };
 
*/








class TableCalendar
{
//
public:
 enum { MAGICNUMBER = 0x42211928, VERSION = 1 };
 TableCalendar();  /* excel flat calendar */
 void InitRange( int mi ,  int max , const int user );  /* year range min , max */
 QString name_m( int m );
 void AppendMonth( int y, int m );
 void AppendYear( int y );
 /* display */
 QList<DayCell> Take( int y  );
 QList<DayCell> Take( int y  , int line );   /* line = month - 1 */
 QList<QDate> ram_day;  /* construct calendar flat each year on table */
 QList<DayCell> visual_day;  /* construct calendar flat each year on table */
 
 inline operator QVariant() const {
    return QVariant::fromValue(*this);
 }
 inline bool isMemo( int epoche  ) 
 {
        if (MemoDay.size() < 1) {
         return false;
        }
             QMapIterator <int,DayCell> i(MemoDay);
                while (i.hasNext()) {
                     i.next();
                      if ( i.key() == epoche ) {
                        return true;
                      }
                 }
                 
   return false;
  }
 
   int anno_min;
   int anno_max;
  
 inline QMap<int,DayCell> Works() { return MemoDay; }
 /* save arguments on stream */
 /* day int from 1.1.1970 to day x + DayCell */
 QMap<int,DayCell> MemoDay;
   int owner;
   QString data;
   QString web;
   QDateTime lastupdate;
};

Q_DECLARE_METATYPE(TableCalendar);

/* salvare i codici delle immagini attributi case hotel ecc... */
inline QString saveCalendar( TableCalendar& xdata  )
{
  QMap<int,DayCell> li = xdata.Works();
  
	QByteArray bytes;
	QBuffer buffer(&bytes);
	if (!buffer.open(QIODevice::WriteOnly)) {
		return QString();
	}
	QDataStream ds(&buffer);
	/* place header */
	ds.setVersion(QDataStream::Qt_4_2);
	ds << (quint32)TableCalendar::MAGICNUMBER;
	ds << (quint32)TableCalendar::VERSION;
	/* place header */
	ds << xdata.owner;
	ds << xdata.data;
  ds << xdata.web;
  ds << xdata.lastupdate;
  
             QApplication::setOverrideCursor(Qt::WaitCursor);
  
            QMapIterator <int,DayCell> i(li);
                while (i.hasNext()) {
                     i.next();
                     DayCell appoint = i.value();
                     ds << appoint;
                 }
             QApplication::restoreOverrideCursor();
	buffer.close();
	return bytes.toBase64();
}



/* decoded base64 stream to put on mysql row , file or network streams */
inline TableCalendar openCalendar( const QString datastream_base64   )
{
	QByteArray xcode("");
  TableCalendar xdata;
	xcode.append(datastream_base64);
	quint32 magic, version;
	QByteArray bytes(QByteArray::fromBase64(xcode));   /* decoded base64 string to QByteArray */
	QBuffer buffer(&bytes);
	if (!buffer.open(QIODevice::ReadOnly)) {
		return xdata;
	}
  
	QDataStream ds(&buffer);
	/* place header */
	ds.setVersion(QDataStream::Qt_4_2);
	ds >> magic;
	if ( (quint32)TableCalendar::MAGICNUMBER != magic ) {
    qDebug() << "######## MAGICNUMBER not ok  ";
		buffer.close();
		return xdata;
	}
	ds >> version;
	if ( (quint32)TableCalendar::VERSION != version ) {
		qDebug() << "######## VERSION not ok  ";
		buffer.close();
		return xdata;
	}
	/* place header */
  ds >> xdata.owner;
	ds >> xdata.data;
  ds >> xdata.web;
  ds >> xdata.lastupdate;
  QApplication::setOverrideCursor(Qt::WaitCursor);
    while (!ds.atEnd()) {
         DayCell appoint;
         ds >> appoint;
         xdata.MemoDay.insert(appoint.fromUday(),appoint);
    }
   QApplication::restoreOverrideCursor();
	buffer.close();
	return xdata;
}








class Calendar : public QTableWidget
{
	Q_OBJECT 
	
public:
	Calendar( QWidget *parent = 0 );
  void init();
  void contextMenuEvent ( QContextMenuEvent * e );
  bool HavingYear( int y );
  void PlayYear( const int InYear);
  TableCalendar taco;
  int MaxY;
  int MinY;
  int ActiveYear;
  public slots:
  void SwapYear();
  void Riservati();
  void Notaday();
  void ExportAsFile();
  void ImportFromFile();

};
















   

















//
#endif // PREANNOSTRUCTURE_H

