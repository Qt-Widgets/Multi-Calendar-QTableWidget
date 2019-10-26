#include "CalendarStructure.h"


DayCell::DayCell()
{
    td_line = 0;
    tr_line = 0;
    modus = 0;
}


QIcon DayCell::createColorToolButtonIcon( int setmodus )
{
    QPixmap pixmap(50, 50);
    pixmap.fill(BG(setmodus));
    QPainter painter(&pixmap);
    return QIcon(pixmap);
}


QPixmap DayCell::NoteIcon()
{
    QPixmap pixmap(5,5);
    pixmap.fill(Qt::darkBlue);
    QPainter painter(&pixmap);
    return pixmap;
}


DayCell DayCell::operator=( const DayCell d )
{
    //////qDebug() << "operator  CDay->" << d.text();
    td_line = d.td_line;
    tr_line = d.tr_line;
    modus = d.modus;
    dax = d.dax;
    note = d.note;
    data = d.data;
    return *this;
}

/*
bool DayCell::operator==( const DayCell &d ) const
{
           if (td_line == d.td_line &&
               tr_line == d.tr_line &&
     dax.toJulianDay() == d.dax.toJulianDay() ) {
      return true;
     } else {
      return false;
     }

}

*/


void DayCell::Register( int tr , int td , const QDate d , int gyear ,  int mod )
{
    td_line = td;
    tr_line = tr;
    modus = mod;
    dax = d;
    anno = gyear;
}

bool DayCell::isValid()
{
    if (modus !=0 || note.size() !=0) {
        return true;
    } else {
        return false;
    }
}


QColor DayCell::BG( int setmodus ) 
{
    /* reservation modus    0= libero / 1= prenotato / 2= prenotato provvisorio / 3= riservato / 10 = chiuso / */

    if ( setmodus == 1 ) {
        return QColor("#ffb4a7");
    } else if ( setmodus == 2 ) {
        return QColor("#ffcf97");
    } else if ( setmodus == 3 ) {
        return QColor("#ff0000");
    } else if ( setmodus == 10 ) {
        return QColor("#bfbfbf");
    } else {
        return QColor(Qt::white);
    }
}

QColor DayCell::FC( int setmodus ) 
{
    /* reservation modus    0= libero / 1= prenotato / 2= prenotato provvisorio / 3= riservato / 10 = chiuso / */

    if ( setmodus == 1 ) {
        return QColor(Qt::darkBlue);
    } else if ( setmodus == 2 ) {
        return QColor(Qt::darkBlue);
    } else if ( setmodus == 3 ) {
        return QColor(Qt::white);
    } else if ( setmodus == 10 ) {
        return QColor(Qt::white);
    } else {
        return QColor(Qt::darkBlue);
    }
}

int DayCell::fromUday()
{
    QDate unixtime = QDate(1970,1,1);
    return dax.toJulianDay()  -  unixtime.toJulianDay();
}

CalendarItem *DayCell::on_Table()
{
    int Weekday = dax.dayOfWeek();

    CalendarItem *item = new CalendarItem(dax);
    if (Weekday == 7 && modus == 0) {
        item->setData (Qt::BackgroundColorRole ,QBrush(QColor("#d2d5ff"),Qt::Dense5Pattern));
    } else {
        item->setData (Qt::BackgroundColorRole ,QBrush(BG(modus )));
    }
    item->setData (Qt::ForegroundRole ,FC(modus) ) ;
    item->setTextAlignment ( Qt::AlignVCenter | Qt::AlignHCenter );

    int mcheck = tr_line + 1;
    if (mcheck == dax.month()) {
        item->setFlags ( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

    } else {
        item->setFlags ( Qt::ItemIsTristate );
        item->setData (Qt::ForegroundRole ,QColor("#e6e6e6")) ;
    }
    QString base = QString("%1 - %2").arg(dax.toString()).arg(fromUday());

    if (note.size() > 0) {
        item->setToolTip ( base + "\n" + note );
        item->setData(Qt::DecorationRole,NoteIcon());
        item->setData(Qt::SizeHintRole,QSize(20,20));
    } else {
        item->setToolTip ( base );
        item->setData(Qt::DecorationRole,QVariant());
        item->setData(Qt::SizeHintRole,sizeHint());
    }




    return item;
}

QString DayCell::text() 
{
    return QString("%1").arg(day());
}

int DayCell::day()
{
    return dax.day();
}

int DayCell::year()
{
    return dax.year();
}





QDataStream& operator<<(QDataStream& out, const DayCell& udoc)
{
    out << udoc.td_line;
    out << udoc.tr_line;
    out << udoc.modus;
    out << udoc.anno;
    out << udoc.dax;
    out << udoc.data;
    out << udoc.note;
    return out;
}

QDataStream& operator>>(QDataStream& in, DayCell& udoc)
{
    in >> udoc.td_line;
    in >> udoc.tr_line;
    in >> udoc.modus;
    in >> udoc.anno;
    in >> udoc.dax;
    in >> udoc.data;
    in >> udoc.note;
    return in;
}

TableCalendar::TableCalendar()
{
    anno_min = 0;
    anno_max = 0;
    owner = 0;
    lastupdate = QDateTime::currentDateTime();
}

QString TableCalendar::name_m( int m )
{
    QStringList RTFmonth = QStringList() << "NULL" << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun" << "Jul" << "Aug" << "Sep" << "Oct" << "Nov" << "Dec";
    return RTFmonth.at(m);
}




void TableCalendar::AppendMonth( int y, int m )
{
    const int row_line = m - 1;    /* cool size - 1 */
    const int NextYear = y + 1;
    QDate first(y,m,1);   /* first day periode */
    QDate next_Y(NextYear,1,1);  /* if M december  append here day */
    const int WeekfirstM = first.dayOfWeek() - 1;
    QDateTime day_1(first);   /* range start */
    QDateTime day_last = day_1.addMonths(1);  /* range stop */
    const int WeeklastM = day_last.date().dayOfWeek();
    int TotalDays = day_1.daysTo(day_last);
    ram_day.clear();
    QStringList debug_visual;
    debug_visual.append(name_m(m)+QString("/%1").arg(y));
    int append = -1;
    /////////qDebug() << "################################################ ";
    /////////qDebug() << "start........ " << name_m(m);

    if (first.dayOfWeek() !=1) {
        append = first.dayOfWeek() - 1;  /* 1 - 6 */
        const int prepend_sum = append;
        while (ram_day.size() < prepend_sum)
        {
            ram_day.append(day_1.addDays(0 - append).date());
            append--;
        }

    } else {
        append = 0;
    }

    /* normal */
    for (int dd = 1; dd <= TotalDays; ++dd) {
        QDate sopra(y,m,dd);
        ram_day.append(sopra);
    }
    append = -1;
    while (ram_day.size() < 35)
    {
        append++;
        if (m == 12) {
            QDateTime newYears = QDateTime(next_Y);
            ram_day.append(newYears.addDays(append).date());
            ////////qDebug() << " nuovo anno   " << m << " " << newYears.date().year();
        } else {
            ram_day.append(day_last.addDays(append).date());
        }

    }
    append = -1;
    for (int i=0; i<35; i++) {
        QDate realdate = ram_day[i];
        DayCell prepare;
        prepare.Register(m - 1,i,realdate,y,0);
        ///////qDebug() << " day line ->" << i << " d." << realdate;
        visual_day.append(prepare);
        debug_visual << QString("%1").arg(realdate.day());
        append++;
    }
    ///////qDebug() << "#### " << debug_visual.join(",");
    ///////
    ///////qDebug() << "#### check real 35 " << append + 1;
    ram_day.clear();
    ////////////qDebug() << "################################################ ";

}





void TableCalendar::AppendYear( int y )
{
    for (int Monthis = 1; Monthis <= 12; ++Monthis) {
        AppendMonth(y,Monthis);
    }
}


QList<DayCell> TableCalendar::Take( int y  )
{
    QList<DayCell> li;
    for (int i=0; i<visual_day.size(); i++) {
        DayCell pin = visual_day[i];
        if (pin.anno == y) {
            li.append(pin);
        }
    }
    return li;
}

QList<DayCell> TableCalendar::Take( int y  , int line )
{
    QList<DayCell> li;
    for (int i=0; i<visual_day.size(); i++) {
        DayCell pin = visual_day[i];
        if (pin.anno == y && pin.tr_line == line) {
            li.append(pin);
        }
    }
    return li;
}




void TableCalendar::InitRange( int mi ,  int max , const int user )
{
    anno_min = qMin(mi,max);
    anno_max = qMax(mi,max);
    lastupdate = QDateTime::currentDateTime();
    owner = user;
    visual_day.clear();

    for (int Years = anno_min; Years <= anno_max; ++Years) {
        AppendYear(Years);
    }

    qDebug() << "#### table item  " << visual_day.size();

    for (int Years = anno_min; Years <= anno_max; ++Years) {
        QList<DayCell> li = Take(Years);
        qDebug() << "#### Years size  " << Years << " " << li.size();
    }




}






Calendar::Calendar( QWidget *parent  )
    : QTableWidget(12,35,parent)
{
    QStringList RTFdays = QStringList() << "Mon" << "Tue" << "Wed" << "Thu" << "Fri" << "Sat" << "Sun";
    QStringList testa;
    testa.clear();
    int dax = 0;
    for (int i=0; i<50; i++) {
        testa.append(RTFdays.at(dax));
        dax++;
        if (dax == 7) {
            dax = 0;
        }
    }

    setMinimumSize (600,310);
    QStringList RTFmonth = QStringList() << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun" << "Jul" << "Aug" << "Sept" << "Oct" << "Nov" << "Dec";
    setVerticalHeaderLabels(RTFmonth);
    setHorizontalHeaderLabels(testa);
    setContextMenuPolicy(Qt::DefaultContextMenu);
    init();
    ///////////setItemDelegate(new DateDelegate());
}

void Calendar::init()
{

    QDate now = QDate::currentDate();
    ActiveYear = now.year();
    MaxY = ActiveYear + 8;
    MinY = ActiveYear - 3;
    taco.InitRange(MinY,MaxY,0);
    setWindowTitle (QString("Year %1 / Object %2 / Last update %3").arg(ActiveYear).arg(taco.owner).arg(taco.lastupdate.toString()));

    for (int line = 0; line <= 11; ++line) {
        QList<DayCell> li = taco.Take(ActiveYear,line);
        for (int x=0; x<li.size(); x++) {
            DayCell pin = li[x];
            CalendarItem *Cell = pin.on_Table();
            Cell->setData(Qt::UserRole,pin);
            setItem(line,x,Cell);
        }
    }
    resizeColumnsToContents();
    resizeRowsToContents();
}


bool Calendar::HavingYear( int y )
{
    for (int Years = MinY; Years <= MaxY; ++Years) {
        if (Years == y) {
            return true;
        }
    }
    return false;
}


void Calendar::SwapYear()
{
    QAction *invoice = qobject_cast<QAction *>(sender());
    int gotoY = invoice->data().toInt();
    if (!HavingYear( gotoY )) {
        return;
    }
    PlayYear(gotoY);
}

void Calendar::PlayYear( const int InYear)
{
    ActiveYear = InYear;
    setWindowTitle (QString("Year %1 / Object %2 / Last update %3").arg(ActiveYear).arg(taco.owner).arg(taco.lastupdate.toString()));

    CalendarItem *Cell;

    for (int line = 0; line <= 11; ++line) {
        QList<DayCell> li = taco.Take(ActiveYear,line);
        for (int x=0; x<li.size(); x++) {
            DayCell pin = li[x];
            if (taco.isMemo( pin.fromUday()  )) {
                ///////////////qDebug() << "trovato  " << pin.fromUday();
                DayCell oldpi = taco.MemoDay.value(pin.fromUday());
                Cell = oldpi.on_Table();
                Cell->setData(Qt::UserRole,oldpi);
            } else {
                Cell = pin.on_Table();
                Cell->setData(Qt::UserRole,pin);
            }

            setItem(line,x,Cell);   ///////////DayCell TableCalendar::isMemo( const DayCell d  )
        }
    }

    resizeColumnsToContents();
    resizeRowsToContents();
    clearSelection();
}

void Calendar::Riservati()
{
    QAction *invoice = qobject_cast<QAction *>(sender());
    int ModusMake = invoice->data().toInt();


    QList<QTableWidgetItem *> selezioni = selectedItems();


    if (selezioni.size() < 0) {
        QMessageBox::warning(0, tr("Error Calendar!"),tr("No Selection state found!"));
        return;
    } else {

        for (int i=0; i<selezioni.size(); i++) {
            DayCell fday = selezioni[i]->data(Qt::UserRole).value<DayCell>();
            fday.modus = ModusMake;
            if (ModusMake == 0) {
                fday.note = "";
            }
            CalendarItem *Cell = fday.on_Table();
            Cell->setData(Qt::UserRole,fday);
            setItem(fday.tr_line,fday.td_line,Cell);
            taco.MemoDay.insert(fday.fromUday(),fday);  /* insert or replace memo !!!! */
        }
    }
    clearSelection();

}


void Calendar::Notaday()
{

    QList<QTableWidgetItem *> selezioni = selectedItems();
    QString notatxt;

    if (selezioni.size() < 0) {
        QMessageBox::warning(0, tr("Error Calendar!"),tr("No Selection state found!"));
        return;
    } else {

        QTableWidgetItem *one = selezioni.first();
        DayCell pday = one->data(Qt::UserRole).value<DayCell>();

        notatxt = QInputDialog::getText(this, tr("Your note (Max 110 char.)"),tr("Note:"), QLineEdit::Normal,pday.note);
        if (notatxt.size() < 1) {
            return;
        }

        for (int i=0; i<selezioni.size(); i++) {
            DayCell fday = selezioni[i]->data(Qt::UserRole).value<DayCell>();

            fday.note = notatxt;
            CalendarItem *Cell = fday.on_Table();
            Cell->setData(Qt::UserRole,fday);
            setItem(fday.tr_line,fday.td_line,Cell);
            taco.MemoDay.insert(fday.fromUday(),fday);  /* insert or replace memo !!!! */
        }
    }

    clearSelection();

}








void Calendar::contextMenuEvent ( QContextMenuEvent * e )
{
    DayCell aday;


    QMenu *TContext = new QMenu();
    QMenu *Tanni = new QMenu(tr("Year %1").arg(ActiveYear));

    for (int Years = MinY; Years <= MaxY; ++Years) {

        if (Years != ActiveYear) {
            QAction *ope = new QAction(this);
            /////ope->setIcon(QIcon(QString::fromUtf8(":/img/kappfinder.png")));
            ope->setText(tr("Open year %1").arg(Years));
            ope->setData ( Years );
            connect(ope, SIGNAL(triggered()), this,  SLOT(SwapYear()));
            Tanni->addAction(ope);
        }
    }

    TContext->addAction(Tanni->menuAction());


    if ( HavingYear( ActiveYear + 1 )) {

        QAction *openext = new QAction(this);
        /////ope->setIcon(QIcon(QString::fromUtf8(":/img/kappfinder.png")));
        openext->setText(tr("Next year >>"));
        openext->setData ( ActiveYear + 1 );
        connect(openext, SIGNAL(triggered()), this,  SLOT(SwapYear()));
        TContext->addAction(openext);
    }
    if ( HavingYear( ActiveYear - 1 )) {

        QAction *openprev = new QAction(this);
        /////ope->setIcon(QIcon(QString::fromUtf8(":/img/kappfinder.png")));
        openprev->setText(tr("Last year <<"));
        openprev->setData ( ActiveYear - 1 );
        connect(openprev, SIGNAL(triggered()), this,  SLOT(SwapYear()));
        TContext->addAction(openprev);
    }


    /* reservation modus    0= libero / 1= prenotato / 2= prenotato provvisorio / 3= riservato / 10 = chiuso / */
    QAction *open0 = new QAction(tr("Make it free"),this);
    open0->setStatusTip(tr("Make it free"));
    open0->setData(0);
    open0->setIcon ( aday.createColorToolButtonIcon(0) );
    connect(open0, SIGNAL(triggered()), this,  SLOT(Riservati()));

    QAction *open1 = new QAction(tr("Booked up"),this);
    open1->setStatusTip(tr("Booked up"));
    open1->setData(1);
    open1->setIcon ( aday.createColorToolButtonIcon(1) );
    connect(open1, SIGNAL(triggered()), this,  SLOT(Riservati()));

    QAction *open2 = new QAction(tr("Provisionally Reserved"),this);
    open2->setStatusTip(tr("Provisionally Reserved"));
    open2->setData(2);
    open2->setIcon ( aday.createColorToolButtonIcon(2) );
    connect(open2, SIGNAL(triggered()), this,  SLOT(Riservati()));

    QAction *open3 = new QAction(tr("Occupied period"),this);
    open3->setStatusTip(tr("Occupied period"));
    open3->setData(3);
    open3->setIcon ( aday.createColorToolButtonIcon(3) );
    connect(open3, SIGNAL(triggered()), this,  SLOT(Riservati()));

    QAction *open10 = new QAction(tr("Closed Period"),this);
    open10->setStatusTip(tr("Closed Period"));
    open10->setData(10);
    open10->setIcon ( aday.createColorToolButtonIcon(10) );
    connect(open10, SIGNAL(triggered()), this,  SLOT(Riservati()));

    TContext->addAction(open0);
    TContext->addAction(open1);
    TContext->addAction(open2);
    TContext->addAction(open3);
    TContext->addAction(open10);
    TContext->addSeparator ();
    TContext->addAction(tr( "Export as file" ), this , SLOT( ExportAsFile() ) );
    TContext->addAction(tr( "Import as file" ), this , SLOT( ImportFromFile() ) );

    TContext->addSeparator ();

    TContext->addAction(tr( "Enter a note ..." ), this , SLOT( Notaday() ) );

    TContext->exec(QCursor::pos());


    open0->deleteLater();
    open1->deleteLater();
    open2->deleteLater();
    open3->deleteLater();
    open10->deleteLater();
    TContext->deleteLater();
    delete TContext;
}

void Calendar::ImportFromFile()
{
    QString fixfile = QFileDialog::getOpenFileName(this, tr("Import - Open file"),
                                                   QCoreApplication::applicationDirPath(),
                                                   "HouseCalendar_file (*.hcal)" );
    if (!fixfile.isEmpty()) {
        QString inside;
        QFile file(fixfile);
        if (file.exists()) {
            if (file.open(QFile::ReadOnly | QFile::Text)) {
                inside = QString::fromUtf8(file.readAll());
                file.close();
            }
        }

        TableCalendar ExternCalendar = openCalendar(inside);
        taco.owner = ExternCalendar.owner;
        taco.data = ExternCalendar.data;
        taco.web = ExternCalendar.web;
        taco.lastupdate = ExternCalendar.lastupdate;
        taco.MemoDay.clear();
        taco.MemoDay = ExternCalendar.Works();

        PlayYear(ActiveYear);  ///////////  Launch years

    }

}

void Calendar::ExportAsFile()
{
    int bigs = taco.MemoDay.size();

    if (bigs < 1) {
        QMessageBox::warning(0, tr("Error Calendar!"),tr("No item found, Select date to reserve..."));
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                    QCoreApplication::applicationDirPath(),
                                                    tr("HouseCalendar_file (*.hcal)"));

    if (!fileName.isEmpty() ) {
        if (!fileName.endsWith(".hcal")) {
            fileName.append(".hcal");
        }
        const QString dati = saveCalendar(taco);
        QTextCodec *codecx;
        codecx = QTextCodec::codecForMib(106);
        QFile f( fileName );
        if ( f.open( QFile::WriteOnly | QFile::Text ) )
        {
            QTextStream sw( &f );
            sw.setCodec(codecx);
            sw << dati;
            f.close();
        }
    }
}








CalendarItem::CalendarItem( const QDate d )
    : QTableWidgetItem()
{
    day = d;
    setText(QString("%1").arg(d.day()) );
    setSizeHint(QSize(10,10));
}

CalendarItem::CalendarItem(const QString &text)
    : QTableWidgetItem(text)
{

}







/*






 QSize DateDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
 {
     if (qVariantCanConvert<DayCell>(index.data(Qt::UserRole))) {
         DayCell dat = qVariantValue<DayCell>(index.data(Qt::UserRole));
         return dat.sizeHint();
     } else {
         return QItemDelegate::sizeHint(option, index);
     }
 }

 
void DateDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
 {
            drawBackground(painter,option,index);

     if (qVariantCanConvert<DayCell>(index.data(Qt::UserRole))) {
            DayCell dat = qVariantValue<DayCell>(index.data(Qt::UserRole));

     } else {
         QItemDelegate::paint(painter, option, index);
     }


}


void DateDelegate::drawBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{


              if (qVariantCanConvert<DayCell>(index.data(Qt::UserRole))) {
            DayCell dat = qVariantValue<DayCell>(index.data(Qt::UserRole));

                        if (dat.dax.dayOfWeek() == 7 && dat.modus !=0 ) {
                            painter->fillRect(option.rect,QBrush(QColor("#e5e5e5"),Qt::BDiagPattern));
                            } else {
                            painter->fillRect(option.rect,dat.BG(dat.modus));
                            painter->drawPixmap (5,5,5,5,dat.NoteIcon());
                            }
                            painter->save();
                            painter->restore();

                } else {
                     QItemDelegate::drawBackground(painter, option, index);
                }

}





*/





