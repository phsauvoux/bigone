# Compiler PhSynth One pour FL Studio sans installer CMake/Visual Studio sur ton PC

Le plus simple est d'utiliser GitHub Actions : GitHub compile le VST3 sur un serveur Windows et te donne un fichier `.zip` à télécharger.

## Étapes

1. Crée un dépôt GitHub vide.
2. Envoie tout le dossier `PhSynthOne` dans ce dépôt.
3. Va dans l'onglet **Actions** du dépôt.
4. Lance le workflow **Build Windows VST3** avec **Run workflow**.
5. Quand le build est fini, télécharge l'artifact **PhSynth-One-Windows-VST3**.
6. Dézippe-le et copie `PhSynth One.vst3` dans :

```text
C:\Program Files\Common Files\VST3
```

7. Ouvre FL Studio > Plugin Manager > Find installed plugins.
