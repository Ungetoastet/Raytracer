# Raytracer

Raytracer - Projekt in CPP für Algorithm Engineering Kurs im WS24/25. Entwickelt von Nele Rissiek und Robert Scheer.

<img src="/Documentation/cornell.png" />

<video width="320" height="240" controls>
  <source src="/Documentation/output_video.mp4" type="video/mp4">
</video>

# Builden

Das Projekt kann mit cmake wie folgt gebuildet werden.
Öffnen Sie ein Terminal im Projekt Ordner.
Dann:

1. `cd Build`
2. `cmake ..`
3. `make`

Danach kann das Programm [ausgeführt werden](README.md#Ausführen).

# Ausführen

Ist das Projekt gebuildet, kann das fertige Programm ausgeführt werden.
Das Programm befindet sich im `Build` Ordner und nimmt zwei Argumente:

`./main <Szene> <Rendersettings>`

Dabei ist `<Scene>` der Pfad zu einer Szenenbeschreibungs-XML (z.B. `/Templates/cornell.scene`) und `<Rendersettings>` der Pfad zu den Render Einstellungen (z.B. `/Templates/settings_default.xml`).

Je nach Rendereinstellungen braucht das Programm unterschiedlich lange zum terminieren, es kann ein wenig Geduld nötig sein.

Gibt das Programm einen `ERROR` zurück, kann die Ausführung abgebrochen werden mit `CTRL+C`.

Gibt das Programm `Writing to file done...` zurück, liegt das fertig gerenderte Bild unter dem in den Rendersettings gegebenen Pfad.

Die PPM-Datei kann in den meisten Bildprogrammen geöffnet werden.
Während der Entwicklung hat sich IrfanView auf Windows und der Standard-Bildbetrachter auf Ubuntu als schnell und zuverlässig erwiesen.

# Einstellungen

Die Einstellungen für den Renderer werden in Form einer XML Datei übergeben.

Eine Vorlage für die Rendersettings Datei kann unter `/Templates/template_settings.xml` gefunden werden.

Die Empfohlenen Einstellungen sind in der `/Templates/settings_default.xml`.

Es sind folgende Einstellungen zur Verfügung:

| Einstellung           | Funktion                                                                                                                                                                                                    |
| --------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| resolution / x        | Breite des Bilds in Pixeln                                                                                                                                                                                  |
| resolution / y        | Höhe des Bilds in Pixeln                                                                                                                                                                                    |
| outputpath / path     | Pfad des gerenderten Bilds                                                                                                                                                                                  |
| depth / b             | Farbtiefe des gerenderten Bilds in bit. Muss entweder 8 oder 16 sein. Eine Tiefe von 16 kann [Color Banding](https://en.wikipedia.org/wiki/Colour_banding) reduzieren, führt aber zu größeren Dateigrößen.  |
| supersampling / steps | Gibt an, wie viele Strahlen pro Bildpixel berechnet werden. Die genaue Anzahl ist steps\*steps. Reduziert Bildrauschen aber hat einen sehr großen Einfluss auf die Programmlaufzeit.                        |
| scatter / base        | Gibt an, in wie viele neue Lichtstrahlen ein Lichtstrahl bei einer Reflektion geteilt wird. Ein höherer Wert reduziert Bildrauschen, beeinflusst aber bei komplexen Szenen die Programmlaufzeit sehr stark. |
| scatter / reduction   | Gibt an, um wie viel der scatter/base Wert pro Reflektion reduziert wird. Ein höherer Wert führt zu schnelleren Renderzeiten, allerdings unter Verlust der Qualität der Reflektionen.                       |
| bounces / count       | Wie oft darf ein einzelner Lichtstrahl maximal reflektiert werden? Erreicht ein Lichtstrahl diese Grenze, wird schwarz zurückgegeben.                                                                       |

# Szene

In der Szenen-XML kann die zu rendernde 3D-Umgebung beschrieben werden.

Eine Vorlage für die Szenen-Struktur kann unter `/Templates/template.scene` gefunden werden.

Ein Beispiel für eine Szenen-Datei ist unter `/Templates/cornell.scene` und stellt eine
[Cornell Box](https://en.wikipedia.org/wiki/Cornell_box) dar.

## Aufbau

Der Aufbau einer Szenen Datei folgt folgendem Muster:

```
scene
├── materials
│ ├── material (1 - n)
├── objects
│ ├── Plane    (0 - n)
│ ├── Sphere   (0 - n)
└── camera     (1)
```

Dabei ist die Reihenfolge wichtig, materials müssen vor objects definiert werden.

## Material

Ein Material ist wie folgt aufgebaut:
`<material id="id" color="r, g, b" reflection="rf" roughness="rn" />`

Dabei ist `id` der Code, um Objekten das Material zuzuweisen.

`color` ist die Grundfarbe des Objekts in RGB.
Dabei sind die Werte im Bereich $[0, 1]$ und können als Dezimalzahl geschrieben werden.

`reflection` gibt an, wie stark das Material spiegelnd reflektiert. Der Wert kann dabei im Bereich $[0, 1]$ sein, wobei ein Wert von $0$ eine vollständig diffuse Reflektion ist und ein Wert von $1$ eine vollständig spiegelnde Reflektion.

`roughness` gibt an, wie rau die spiegelnde Reflektion ist. Ein Wert von $0$ ist dabei eine perfekte Reflektion, z.B. für Spiegel oder glattes Plastik.
Die `roughness` darf maximal einen Wert von $1$ annehmen.

## Objects

Es gibt zwei Arten von Objekten: Platten und Kugeln. Es können beliebig viele Objekte in einer Szene vorhanden sein.

### Plane

Eine Plane ist eine rechteckige, gerade Platte.

Eine Plane ist wie folgt aufgebaut:
`<Plane position="x, y, z" rotation="rx, ry, rz" scale="sx, sy, sz" material="id">`

`position` ist die Position der Platte in kartesischen Koordinaten. `x, y, z` können dabei jeden beliebigen Wert annehmen, in Dezimaldarstellung.

`rotation` ist die Rotation bzw. Orientierung der Platte im Raum. Bei einer Rotation von `0, 0, 0` liegt die Platte parallel zur x-y-Ebene.
Die Werte der Rotation sind in Eulerschen Winkeln und können dabei jeden beliebigen Wert in Dezimalzahlen annehmen.

`scale` ist die Größe der Platte. `sx, sy, sz` können dabei jeden beliebigen Wert $>0$ in Dezimaldarstellung annehmen. Statt `scale="x, x, x"` kann auch `size="x"` geschrieben werden.

`material` ist das Material auf der Platte. Das Material muss vorher definiert sein im `Materials`-Abschnitt und die id muss exact übereinstimmen.

### Sphere

Eine Sphere ist eine perfekte Kugel.

Eine Sphere ist wie folgt aufgebaut:
`<Sphere position="x, y, z" radius="r" material="id" />`

`position` ist die Position der Kugel in kartesischen Koordinaten. `x, y, z` können dabei jeden beliebigen Wert annehmen, in Dezimaldarstellung.

`radius` ist der Radius der Kugel.

`material` ist das Material auf der Kugel. Das Material muss vorher definiert sein im `Materials`-Abschnitt und die id muss exact übereinstimmen.

## Lichter

Ein Licht kann durch ein Material definiert werden. Dabei muss der `roughness` Wert $=-1$ sein.

Dann wird das `color`-Attribut als Lichtfarbe verwendet. Es macht Sinn, für die Komponenten der `color` Werte $>1$ zu verwenden. Größere Werte sorgen für ein helleres Licht.

## Camera

Jede Szene muss exakt eine Kamera beinhalten.
Eine Kamera ist wie folgt definiert:
`<camera position="x, y, z" lookAt="lx, ly, lz", fieldOfView="fov" skybox="s" />`.

`position` ist die Position der Kamera in kartesischen Koordinaten. `x, y, z` können dabei jeden beliebigen Wert annehmen, in Dezimaldarstellung.

`lookAt` gibt die Position im Koordinatensystem an, auf die die Kamera zentriert ist. Diese Koordinate ist im gerenderten Bild in der Mitte des Bildes. `lx, ly, lz` können dabei jeden beliebigen Wert annehmen, in Dezimaldarstellung.

`lookAt` darf nicht gleich `position` sein.

`fieldOfView` gibt das vertikale Sichtfeld der Kamera in Grad an. Ein Wert von 45 wird empfohlen.

`skybox` gibt an, wie sich der Hintergrund der Szene verhält. Ist `skybox="false"`, ist der Hintergrund der Szene schwarz. Ist `skybox="true"` ist der Hintergrund der Szene ein Farbverlauf, der dem eines Sonnenuntergangs ähnelt.

# Film

Ist [python](https://www.python.org/downloads/) und [ffmpeg](https://www.ffmpeg.org/download.html) installiert, kann mit `python physicsmovie.py` ein kleiner mp4 Film gerendert werden.

Dabei wird in Python eine einfache Physiksimulation einfacher Kugeln durchgeführt. Für jedes Zeitschritt werden die Positionen der Kugeln in einer Szenen-Datei ausgegeben und dann der Renderer ausgeführt.

Die gerenderten Bilder werden dann mit ffmpeg zu einem Film zusammengeführt.

Eine Version eines solchen Films ist unter `/Documentation/output_video.mp4` zu finden.
