# Stichpunkte für das Paper

## Generelles Zeug

-   Motivation:

    -   Sieht cool aus?
    -   Sehr gut parallelisierbar (Aber deshalb eigentlich besser auf GPU)

-   Related Work:

    -   https://raytracing.github.io/books/RayTracingInOneWeekend.html

-   Einschränkungen:
    -   Nicht bestmögliche Performance wegen CPU statt GPU
    -   Kein Laden von 3D Modellen, nur primitive Formen
        -   Primitive Formen (Platte, Kugel) lassen sich mathematisch Definieren statt über Punktmengen, deutlich weniger stressig

## Algorithmus

-   3D-Scene in XML definiert

    -   Eigener XML-Parser geschrieben
    -   XML ist von Menschen lesbar aber auch automatisch erstellbar
    -   Endlose Möglichkeiten, halbwegs übersichtlich

-   Render Einstellungen in XML definiert

    -   Klares Format
    -   Menschen und Maschinen lesbar

-   3D-Scene wird dann erst in OOP-Struktur dargestellt, dann aber manuell in den Speicher gelegt

    -   OOP-Casting und Datensuche (wenn man halt auf die Eigenschaften der Objekte zugreift) sehr teuer, vorallem bei den Milliarden Funktionsaufrufen beim Raytracer
    -   In der OOP-Struktur lassen sich Werte einfach ändern, DO-Struktur (DO = Data Oriented) deutlich performanter
    -   Performance tests und details siehe [dokumentation](data_oriented.md)

-   Mit der 3D-Scene werden auch die Vektoren für Positionen und Richtungen von der OOP-Struktur zur \_m128 vector intrunsics Struktur geändert.

-   Performance details siehe [dokumentation](vectorization.md)

-   Das Vehalten jedes Pixels wird durch eine Kernelfunktion bestimmt

    -   Ermöglicht schnelles Testen von verschiedenen Kernels mit unterschiedlichem Verhalten, super zum Debuggen von neuen Features
    -   Beispielsweise sind die Kernels: Volles Raytracing, Raytracing ohne Reflektionen, nur Normalenvektoren anzeigen, Abstand zur Kamera anzeigen

-   Beim Rendern wird für jeden Pixel der Kernel aufgerufen
-   Ab hier geht es um den Finalen Raytracing-Kernel
-   Für jeden Pixel werden eine Gruppe gleich verteilter Rays erstellt (Stichwort Supersampling)
    -   Das Glättet Kanten (Stichwort Aliasing) und
    -   Reduziert Rauschen (Noise)
    -   Nicht performance optimal, es gibt bessere Anti-Aliasing und De-Noising Algorithmen, allerdings ist supersampling sehr einfach zu implementieren
-   Für jeden Ray startet jetzt die Rekursion

    -   Die Maximale Rekursionstiefe ist durch die Anzahl der Bounces definiert
        -   Bounces: Wie oft darf ein einzelner Strahl reflektiert werden?
    -   Der Object Memory wird jetzt durchlaufen und alle Kollisionen berechnet
    -   Die nähste Kollision zum Ursprung des Rays wird ausgewählt
    -   Die Farbe des Objekts wird zur Farbe des Rays addiert
    -   Je nach Qualitätseinstellung (Scatter = Anzahl der erstellten Rays) werden jetzt X neue Rays nach folgendem Muster erstellt:
    -   Es wird ein neuer Ray erstellt, der Ursprung ist am Punkt der Kollision
    -   Die Richtung wird je nach Material des Objekts entweder durch Reflektion an der Normalen berechnet, für Glänzende Materialien oder durch diffuse Reflektion (Stichwort Lambertian Reflection)
    -   Zur Berechneten Richtung kommt eine Zufallskomponente hinzu, die Stärke der Zufallskomponente ist abhängig von der Mattheit des Materials.
        -   Eine kleinere Zufallskomponente = Glatteres Material (zB Spiegel)
        -   Eine größere Zufallskomponente = Raueres Material (zB Gebürstetes Aluminium)
    -   Für jeden erstellten Ray wird eine neue Rekursion gestartet
    -   Die Anzahl der erstellten Rays wird reduziert für jeden Rekursionsaufruf
        -   Je tiefer man geht in der Rekursion, desto weniger Einfluss hat der Ray auf den gerenderten Pixel
        -   Wieso also für minimale Änderungen am Resultat super viele Rays spawnen?
        -   Fix durch Scatter-Reduktion

-   Ausgabe der gerenderten Daten im ASCII-PPM (Portable Pixmap) Format

    -   RGB Werte jedes Pixels einfach nacheinander in einer Textdatei
    -   Sehr einfach zu kodieren
    -   Menschen-lesbar
    -   Kompressionslos -> Kein Informationsverlust
    -   Leicht weiter-verarbeitbar

-   Laufzeit des Renders Abhängig von:

    -   Anzahl der Objekte (Linear)
    -   Auflösung des gerenderten Bilds (Linear)
    -   Einstellung: Supersampling (Quadratisch)
    -   Einstellung: Bounces (Exponentiell)
    -   Einstellung: Scatter (Glaub auch Exponentiell)
    -   Einstellung: Scatter-Reduktion (Invers exponentiel???)

-   Animation über komplett getrenntes Python Programm
    -   Einfache Physik Simulation
    -   Exportiert die Positionen der Körper in das Scene XML Format
    -   Startet den Renderer
    -   Nimmt das gerenderte PPM-Bild und fügt es dem Video mithilfe von ffmpeg hinzu
    -   Output als MP4
