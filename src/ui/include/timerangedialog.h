#ifndef TIMERANGEDIALOG_H
#define TIMERANGEDIALOG_H

#include <QDateTime>
#include <QDialog>
#include <QtGlobal>

class QDateTimeEdit;

// Dialogo para pedir un rango horario "desde / hasta".
// El "hasta" incluye el segundo completo (HH:MM:SS.999).
class TimeRangeDialog : public QDialog {
  public:
    explicit TimeRangeDialog( QWidget* parent = nullptr );

    void setRange( const QDateTime& from, const QDateTime& to );

    // Valores en la misma escala que TimeRangeFinder::toMsecs.
    qint64 fromMs() const;
    qint64 toMs() const;

    void accept() override;

  private:
    QDateTimeEdit* fromEdit_;
    QDateTimeEdit* toEdit_;
};

#endif
