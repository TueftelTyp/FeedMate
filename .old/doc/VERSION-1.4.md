# FeedMate v1.4

Diese Version ordnet die Smartphone-Oberfläche neu und macht mehrere Automaten zu einer gemeinsam bedienbaren Gruppe.

## Bedienablauf

1. Unter **Einstellungen** jedem Automaten einen Namen geben und denselben Gruppenschlüssel mit mindestens 16 Zeichen speichern.
2. Unter **Futter & Kalibrierung** den Probelauf starten, das ausgegebene Futter extern wiegen und das Profil speichern. Für Gruppenaktionen auf allen Automaten denselben Futternamen verwenden; Messwerte dürfen abweichen.
3. In **Automaten** die übrigen lokalen Adressen und den Gruppenschlüssel einmalig hinzufügen.
4. Gewünschte Ziele auswählen und gemeinsam füttern oder den aktuell geladenen Zeitplan übertragen.

Die Statuslampe zeigt den lokalen Zustand und den zuletzt abgefragten Gruppenstatus. Grün bedeutet keinen erkannten Fehler, Gelb eine Warnung und Rot einen kritischen oder nicht erreichbaren Automaten. Sie ersetzt keine mechanische oder elektrische Sicherheitsabschaltung.

## Datenschutz und Netzgrenzen

Remote-Adressen und Gruppenschlüssel werden im lokalen Browser-Speicher des Smartphones abgelegt. Browserprofil und Smartphone müssen daher geschützt sein. FeedMate verwendet weiterhin unverschlüsseltes HTTP/MQTT und gehört nur in ein vertrauenswürdiges lokales Netz. Keine Router-Portfreigabe einrichten.

Die Zeitpläne werden nicht unbemerkt im Hintergrund synchronisiert: Die Schaltfläche **Auf ausgewählte Automaten übertragen** verteilt den sichtbaren Plan bewusst an die ausgewählten entfernten Geräte. Ein fehlgeschlagenes Ziel wird gemeldet, erfolgreiche Ziele bleiben aktualisiert.
