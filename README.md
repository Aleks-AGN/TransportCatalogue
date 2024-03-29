# TransportCatalogue

TransportCatalogue - транспортный справочник для хранения транспортных маршрутов и обработки запросов.

## Возможности

* Поддержка JSON. Справочник считывает структуру базы данных и запросы, и выводит ответ в формате JSON-объектов.

* Визуализация. В справочнике реализована визуализация карты автобусных маршрутов в формате SVG-объектов.

* Маршрутизация. Реализовано построение оптимальных по времени маршрутов между остановками.

* Сериализация. Реализованы создание базы транспортного справочника по запросам и её сериализация в файл, и десериализация базы из файла и использование её для ответов на запросы. Для сериализации и десериализации транспортного справочника применяется Google Protocol Buffers(Protobuf).

## Требования

* C++17 и выше
* Protobuf 3
