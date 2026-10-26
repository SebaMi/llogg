#ifndef TIMERANGEFINDER_H
#define TIMERANGEFINDER_H

#include <functional>
#include <optional>

#include <QDate>
#include <QString>
#include <QTime>
#include <QtGlobal>

#include "abstractlogdata.h"

// Convierte un rango horario "desde / hasta" en un rango de lineas
// [start, end) del log, para usarlo con LogFilteredData::runSearch.
//
// Reglas:
//  - Las lineas sin timestamp (stack traces, mensajes multilinea) heredan el
//    timestamp de la linea anterior que si lo tiene.
//  - Supone que el log esta en orden cronologico (busqueda binaria).
//  - 'end' es exclusivo, igual que el endLine de runSearch.
class TimeRangeFinder {
  public:
    // Devuelve los milisegundos del timestamp de una linea, o nullopt si la
    // linea no tiene un timestamp reconocible.
    using Parser = std::function<std::optional<qint64>( const QString& )>;

    struct Result {
        bool valid = false;
        qint64 start = 0; // primera linea del rango (indice desde 0)
        qint64 end = 0;   // una pasada la ultima linea del rango
        QString error;    // motivo cuando valid == false
    };

    // Parser por defecto: acepta "2026-10-09 14:32:01", "2026-10-09T14:32:01",
    // con fraccion opcional (",123" o ".123"), buscado al inicio de la linea.
    static std::optional<qint64> defaultParser( const QString& line );

    // Escala comun para comparar: milisegundos desde un origen fijo.
    // Usar para convertir el "desde / hasta" del usuario.
    static qint64 toMsecs( const QDate& date, const QTime& time );

    static Result find( const AbstractLogData& data, qint64 fromMs, qint64 toMs,
                        const Parser& parser = &TimeRangeFinder::defaultParser );
};

#endif
