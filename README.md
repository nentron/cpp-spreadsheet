# cpp-spreadsheet
### Описание проекта
Электронная таблица -  упрощёно реализует Microsoft Excel или Google Sheets. Таблица гарантирует создание, удаление, редактирования и контроль валидности значений ячеек таблицы.  Ячейка таблицы будет пустой, текстом, цифрой или формулой.
### Технологии
С++17
STL
ANTL
Unit Tests
CMake
### Сборка
- Скачать данный проект
- Скачать файлы Complete ANTLR 4.13.1 Java binaries jar и source distribution для С++ из https://www.antlr.org/download.html
- Разместь Java binaries jar файл в папке spreadsheet
- Создать папку antlr4_runtime и распаковать содержимое второго файла в созданную папку
- Выполнить команду находясь в директории spreadsheet:
```
 mkdir build && cd build
 cmake .. -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles"
 cmake --build .
```
### Автор
@nentron