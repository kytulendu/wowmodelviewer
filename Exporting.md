< <a href='http://code.google.com/p/wowmodelviewer/wiki/Home'>Home</a> ><
<a href='http://code.google.com/p/wowmodelviewer/wiki/ToDo'>To Do</a> ><
<a href='http://code.google.com/p/wowmodelviewer/wiki/Exporting'>Model Exporting</a> ><
<a href='http://wowmodelviewer.googlecode.com/svn/trunk/doc/buildnotes.txt'>Build Notes</a> ><
<a href='http://wowmodelviewer.googlecode.com/svn/trunk/doc/ChangeLog.txt'>Changelog</a> >

![http://i43.tinypic.com/25zh65e.jpg](http://i43.tinypic.com/25zh65e.jpg)

For a very detailed graph of what WMV can and can't do, please check <a href='http://www.wowmodelviewer.org/forum/index.php?topic=5784.0'>this forum topic</a>.

# What WoW Model Viewer Can Do #
WoW Model Viewer (WMV) can export various types of models to various different formats. Not every exporter works 100% with all the models. That is our eventual goal, but we aren't there yet.

## Model Types ##
There are three types of models in World of Warcraft. WMV can read 2 of them; Model (.m2) files, and World Model Object (.wmo) files. The third type of model is Terrain (.adt) files.

**M2s** are the barrels, tables, rugs, and other misc items that you see. They are also the player models, the monsters you fight and even the spells you cast!

**WMOs** are cities, bridges, dungeons, really anything your character can collide with. WMOs often contain M2 models as Doodads. (Items that decorate rooms, and makes the WMO more interesting to look at.)

## Model Formats ##
Currently, WMV can export models to the following formats:
  * <a href='http://www.newtek.com/lightwave/'>Lightwave 3D</a>: M2 & WMO files
  * <a href='http://en.wikipedia.org/wiki/Obj'>Waveform OBJ file</a>: M2 & WMO files
  * <a href='http://chumbalum.swissquake.ch/'>Milkshape 3D</a>: M2 files only
  * <a href='http://usa.autodesk.com/adsk/servlet/pc/index?siteID=123112&id=6837478'>Autodesk FBX</a>: M2 files only
  * <a href='http://usa.autodesk.com/adsk/servlet/pc/index?id=13567410&siteID=123112'>Autodesk 3DS file</a>: M2 files only
  * <a href='http://www.web3d.org/x3d/'>X3D & X3D in XHTML</a>: M2 files only

## Exporter Specific Capabilties ##
### Lightwave 3D ###
  * Export WMOs with Lights and Objects/Nulls in the proper locations
  * Export of Items Worn by Players (Weapons, helmets, Etc.)
  * Bones Export
  * Export Cameras
  * Export Camera Animation
### Milkshape 3D ###
  * Bones Export
  * Export of Items Worn by Players

# What WoW Model Viewer Can't Do #
## M2 Exporter Limitations ##
M2 Exporters can't do the following:
  * Export Bones (Exeption: Lightwave 3D, FBX & Milkshape 3D)
  * Export Animation (Exeption: FBX)
  * EXport Cameras & Camera Animation (Exception: Lightwave 3D)
  * Export of Items Worn by Players Perfectly

## WMO Exporter Limitations ##
WMO Exporters can't do the following:
  * Export Vertex Lighting/Colors (Exeption: Lightwave 3D)
  * Export Lights (Exeption: Lightwave 3D)
  * Export Doodad Objects/Nulls (Exeption: Lightwave 3D)

## Known Exporter Problems ##
3DS Exporter:
  * **Outputs Invalid File**: Actually, WMV outputs the data just fine. But something in the file causes it to be reported as an invalid file. We are currently looking into it. Oddly enough, Lightwave 3D imports WMV's 3DS files without any problems...

Milkshape 3D Exporter:
  * **Textures aren't applied/attached to the model**.
  * **Exported Bones don't support the Init Pose**.

Lightwave 3D Exporter:
  * 