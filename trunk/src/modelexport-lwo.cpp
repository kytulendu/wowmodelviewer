#include <wx/wfstream.h>

#include "globalvars.h"
#include "modelexport.h"
#include "lwoheader.h"
#include "modelcanvas.h"

#include "CxImage/ximage.h"

//---------------------------------------------
// Scene Writing Functions
//---------------------------------------------

// Writes a single Key for an envelope.
void WriteLWSceneEnvKey(ofstream &fs, unsigned int Chan, float value, float time, unsigned int spline = 0)
{
	fs << "  Key ";				// Announces the start of a Key
	fs << value;				// The Key's Value;
	fs << " " << time;			// Time, in seconds, a float. This can be negative, zero or positive. Keys are listed in the envelope in increasing time order.
	fs << " " << spline;		// The curve type, an integer: 0 - TCB, 1 - Hermite, 2 - 1D Bezier (obsolete, equivalent to Hermite), 3 - Linear, 4 - Stepped, 5 - 2D Bezier
	fs << " 0 0 0 0 0 0 \n";	// Curve Data 1-6, all 0s for now.
}

// Writes an entire channel with only 1 key.
// Use WriteLWSceneEnvArray for writing animations.
void WriteLWSceneEnvChannel(ofstream &fs, unsigned int ChanNum, float value, float time, unsigned int spline = 0)
{
	fs << "Channel " << ChanNum << "\n";	// Channel Number
	fs << "{ Envelope\n";
	fs << "  1\n";						// Number of Keys in this envelope.
	WriteLWSceneEnvKey(fs,ChanNum,value,time,spline);
	fs << "  Behaviors 1 1\n";			// Pre/Post Behaviors. Defaults to 1 - Constant.
	fs << "}\n";
}

// Used for writing the keyframes of an animation.
void WriteLWSceneEnvArray(ofstream &fs, unsigned int count, unsigned int ChanNum, float value[], float time[], unsigned int spline[])
{
	fs << "Channel " << ChanNum << "\n";
	fs << "{ Envelope\n";
	fs << "  " << count << "\n";
	for (int n=0;n<count;n++){
		unsigned int thisspline = 0;
		if (spline[n]) {
			thisspline = spline[n];
		}
		WriteLWSceneEnvKey(fs,ChanNum,value[n],time[n],thisspline);
	}

	fs << "  Behaviors 1 1\n";
	fs << "}\n";
}

// Writes the "Plugin" information for a scene object, light, camera &/or bones.
void WriteLWScenePlugin(ofstream &fs, wxString type, int PluginCount, wxString PluginName, wxString Data = "")
{
	fs << "Plugin " << type << " " << PluginCount << " " << PluginName << "\n" << Data << "EndPlugin\n";
}

// Writes an Object or Null Object to the scene file.
void WriteLWSceneObject(ofstream &fs, wxString Filename, Vec3D Pos, Vec3D Rot, float Scale, int &ItemNumber, bool isNull = false, int ParentNum = -1, wxString Label="")
{
	bool isLabeled = false;
	bool isParented = false;
	if (Label!="")
		isLabeled = true;
	if (ParentNum > -1)
		isParented = true;

	if (isNull == true){
		fs << "AddNullObject";
	}else{
		fs << "LoadObjectLayer 1";
	}
	fs << " 1" << wxString::Format("%07x",ItemNumber) << " " << Filename << "\nChangeObject 0\n";
	if (isLabeled)
		fs << "// " << Label << "\n";
	fs << "ShowObject 7 -1 0.376471 0.878431 0.941176 \nGroup 0\nObjectMotion\nNumChannels 9\n";
	// Position
	WriteLWSceneEnvChannel(fs,0,Pos.x,0);
	WriteLWSceneEnvChannel(fs,1,Pos.y,0);
	WriteLWSceneEnvChannel(fs,2,-Pos.z,0);
	// Rotation
	WriteLWSceneEnvChannel(fs,3,Rot.x,0);
	WriteLWSceneEnvChannel(fs,4,Rot.y,0);
	WriteLWSceneEnvChannel(fs,5,Rot.z,0);
	// Scale
	WriteLWSceneEnvChannel(fs,6,Scale,0);
	WriteLWSceneEnvChannel(fs,7,Scale,0);
	WriteLWSceneEnvChannel(fs,8,Scale,0);

	fs << "PathAlignLookAhead 0.033\nPathAlignMaxLookSteps 10\nPathAlignReliableDist 0.001\n";
	if (isParented)
		fs << "ParentItem 1" << wxString::Format("%07x",ParentNum) << "\n";
	fs << "IKInitialState 0\nSubPatchLevel 3 3\nShadowOptions 7\n";

	fs << "\n";
	ItemNumber++;
}

// Write a Light to the Scene File
void WriteLWSceneLight(ofstream &fs, int &lcount, Vec3D LPos, unsigned int Ltype, Vec3D Lcolor, float Lintensity, bool useAtten, float AttenEnd, float defRange = 2.5, wxString prefix = "", int ParentNum = NULL)
{
	bool isParented = false;
	if (ParentNum!=NULL)
		isParented = true;
	if (prefix != "")
		prefix = " "+prefix;
	if ((useAtten == true)&&(AttenEnd<=0))
		useAtten = false;


	fs << "AddLight 2" << wxString::Format("%07x",lcount) << "\n";
	//modelname[0] = toupper(modelname[0]);
	fs << "LightName " << prefix << "Light " << lcount+1 << "\n";
	fs << "ShowLight 1 -1 0.941176 0.376471 0.941176\n";	// Last 3 Numbers are Layout Color for the Light.
	fs << "LightMotion\n";
	fs << "NumChannels 9\n";
	// Position
	WriteLWSceneEnvChannel(fs,0,LPos.x,0);
	WriteLWSceneEnvChannel(fs,1,LPos.y,0);
	WriteLWSceneEnvChannel(fs,2,-LPos.z,0);
	// Rotation
	WriteLWSceneEnvChannel(fs,3,0,0);
	WriteLWSceneEnvChannel(fs,4,0,0);
	WriteLWSceneEnvChannel(fs,5,0,0);
	// Scale
	WriteLWSceneEnvChannel(fs,6,1,0);
	WriteLWSceneEnvChannel(fs,7,1,0);
	WriteLWSceneEnvChannel(fs,8,1,0);

	if (isParented)
		fs << "ParentItem 1" << wxString::Format("%07x",ParentNum) << "\n";

	// Light Color Reducer
	// Some lights have a color channel greater than 255. This reduces all the colors, but increases the intensity, which should keep it looking the way Blizzard intended.
	while ((Lcolor.x > 1)||(Lcolor.y > 1)||(Lcolor.z > 1)) {
		Lcolor.x = Lcolor.x * 0.99;
		Lcolor.y = Lcolor.y * 0.99;
		Lcolor.z = Lcolor.z * 0.99;
		Lintensity = Lintensity / 0.99;
	}

	fs << "LightColor " << Lcolor.x << " " << Lcolor.y << " " << Lcolor.z << "\n";
	fs << "LightIntensity " << Lintensity << "\n";

	// Process Light type & output!
	switch (Ltype) {
		// Omni Lights
		case 1:
		default:
			// Default to an Omni (Point) light.
			fs << "LightType 1\n";

			if (useAtten == true) {
				// Use Inverse Distance for the default Light Falloff Type. Should better simulate WoW Lights, until I can write a WoW light plugin for Lightwave...
				fs << "LightFalloffType 2\nLightRange " << AttenEnd << "\n";
			}else{
				// Default to these settings, which look pretty close...
				fs << "LightFalloffType 2\nLightRange " << defRange << "\n";
			}
			fs << "ShadowType 1\nShadowColor 0 0 0\n";
			WriteLWScenePlugin(fs,"LightHandler",1,"PointLight");
	}
	fs << "\n";
	lcount++;
}


//---------------------------------------------
// --==M2==--
//---------------------------------------------

/* LWO2

Links to helpful documents:
http://gpwiki.org/index.php/LWO
http://home.comcast.net/~erniew/lwsdk/docs/filefmts/lwo2.html


NOTE!!
I've done some research into the LWO2 format. I have a HUGE commented section about it down in the WMO function that details the layout, and some of the byte info too.
I'll update this function once I re-tune the WMO function.
		-Kjasi
*/
void ExportM2toLWO(Attachment *att, Model *m, const char *fn, bool init)
{
	wxString file = wxString(fn, wxConvUTF8);

	if (modelExport_LW_PreserveDir == true){
		wxString Path, Name;

		Path << file.BeforeLast('\\');
		Name << file.AfterLast('\\');

		MakeDirs(Path,"Objects");

		file.Empty();
		file << Path << "\\Objects\\" << Name;
	}
	if (m->modelType != MT_CHAR){
		if (modelExport_PreserveDir == true){
			wxString Path1, Path2, Name;
			Path1 << file.BeforeLast('\\');
			Name << file.AfterLast('\\');
			Path2 << wxString(m->name.c_str()).BeforeLast('\\');

			MakeDirs(Path1,Path2);

			file.Empty();
			file << Path1 << '\\' << Path2 << '\\' << Name;
		}
	}

	wxFFileOutputStream f(file, wxT("w+b"));

	if (!f.IsOk()) {
		wxLogMessage(_T("Error: Unable to open file '%s'. Could not export model."), file);
		return;
	}

	int off_t;
	uint32 counter=0;
	uint32 TagCounter=0;
	uint16 PartCounter=0;
	uint16 SurfCounter=0;
	unsigned short numVerts = 0;
	unsigned short numGroups = 0;
	uint16 dimension = 2;

	// LightWave object files use the IFF syntax described in the EA-IFF85 document. Data is stored in a collection of chunks. 
	// Each chunk begins with a 4-byte chunk ID and the size of the chunk in bytes, and this is followed by the chunk contents.

	//InitCommon(att, init);

	unsigned int fileLen = 0;

	// ===================================================
	// FORM		// Format Declaration
	//
	// Always exempt from the length of the file!
	// ===================================================
	f.Write("FORM", 4);
	f.Write(reinterpret_cast<char *>(&fileLen), 4);


	// ===================================================
	// LWO2
	//
	// Declares this is the Lightwave Object 2 file format.
	// LWOB is the first format. It doesn't have a lot of the cool stuff LWO2 has...
	// ===================================================
	f.Write("LWO2", 4);
	fileLen += 4;


	// ===================================================
	// TAGS
	//
	// Used for various Strings. Known string types, in order:
	//		Sketch Color Names
	//		Part Names
	//		Surface Names
	// ===================================================
	f.Write("TAGS", 4);
	uint32 tagsSize = 0;
	wxString TAGS;
	u32 = 0;
	f.Write(reinterpret_cast<char *>(&u32), 4);
	fileLen += 8;

#ifdef _DEBUG
	// Debug Texture List
	wxLogMessage("M2 Texture List for %s:",wxString(m->fullname));
	for (unsigned short i=0; i<m->TextureList.size(); i++) {
		wxLogMessage("Texture List[%i] = %s",i,wxString(m->TextureList[i]));
	}
	wxLogMessage("M2 Texture List Complete for %s",wxString(m->fullname));
#endif

	// Mesh & Slot names
	wxString meshes[19] = {_T("Hairstyles"), _T("Facial1"), _T("Facial2"), _T("Facial3"), _T("Braces"),
		_T("Boots"), _T(""), _T("Ears"), _T("Wristbands"),  _T("Kneepads"), _T("Pants"), _T("Pants"),
		_T("Tarbard"), _T("Trousers"), _T(""), _T("Cape"), _T(""), _T("Eyeglows"), _T("Belt") };
	wxString slots[15] = {_T("Helm"), _T(""), _T("Shoulder"), _T(""), _T(""), _T(""), _T(""), _T(""),
		_T(""), _T(""), _T("Right Hand Item"), _T("Left Hand Item"), _T(""), _T(""), _T("Quiver") };

	// Texture Array
	wxString *texarray = new wxString[m->header.nTextures+3];
	for (unsigned short i=0; i<m->passes.size(); i++) {
		ModelRenderPass &p = m->passes[i];

		if (!texarray[p.tex]){
			bool nomatch = true;
			for (int t=0;t<m->TextureList.size();t++){
				if (t == p.tex){
					int n = t-1;
					if (m->modelType != MT_NPC){
						n++;
					}
					texarray[p.tex] = wxString(m->TextureList[n].c_str()).BeforeLast('.');
					if ((m->modelType != MT_CHAR)&&(texarray[p.tex] == "Cape")){
						texarray[p.tex] = wxString(fn, wxConvUTF8).AfterLast('\\').BeforeLast('.') + "_Replacable";
					}
					//wxLogMessage("Texture %i is %s",t,texarray[p.tex]);
					nomatch = false;
					break;
				}
			}
			if (nomatch == true){
				texarray[p.tex-1] = wxString(m->name.c_str()).BeforeLast('.') << wxString::Format(_T("_Material_%03i"), p.tex);
			}
		}
	}
	
	wxString *texarrayAtt;
	wxString *texarrayAttc;
	if (att!=NULL){
		Model *attM = NULL;
		if (att->model) {
			attM = static_cast<Model*>(att->model);

			if (attM){
				texarrayAtt = new wxString[attM->header.nTextures+3];
				for (size_t i=0; i<attM->passes.size(); i++) {
					ModelRenderPass &p = attM->passes[i];

					bool nomatch = true;
					for (int t=0;t<attM->TextureList.size();t++){
						if ((t == p.tex)&&(texarrayAtt[p.tex] != "(null)")){
							texarrayAtt[p.tex] = wxString(attM->TextureList[t].c_str()).BeforeLast('.');
							if (texarray[p.tex] == "Cape"){
								texarray[p.tex] = wxString(fn, wxConvUTF8).AfterLast('\\').BeforeLast('.') + "_Replacable";
							}
							nomatch = false;
							break;
						}
					}
					if (nomatch == true){
						texarrayAtt[p.tex] = wxString(attM->name.c_str()).BeforeLast('.') << wxString::Format(_T("_Material_%03i"), p.tex);
					}
				}
			}
		}

		for (size_t i=0; i<att->children.size(); i++) {
			Attachment *att2 = att->children[i];
			for (size_t j=0; j<att2->children.size(); j++) {
				Model *mAttChild = static_cast<Model*>(att2->children[j]->model);

				if (mAttChild){
					texarrayAttc = new wxString[mAttChild->header.nTextures+3];
					for (size_t i=0; i<mAttChild->passes.size(); i++) {
						ModelRenderPass &p = mAttChild->passes[i];

						bool nomatch = true;
						for (int t=0;t<mAttChild->TextureList.size();t++){
							if ((t == p.tex)&&(texarrayAttc[p.tex] != "(null)")){
								texarrayAttc[p.tex] = wxString(mAttChild->TextureList[t].c_str()).BeforeLast('.');
								if (texarray[p.tex] == "Cape"){
									texarray[p.tex] = wxString(fn, wxConvUTF8).AfterLast('\\').BeforeLast('.') + "_Replacable";
								}
								nomatch = false;
								break;
							}
						}
						if (nomatch == true){
							texarrayAttc[p.tex] = wxString(mAttChild->name.c_str()).BeforeLast('.') << wxString::Format(_T("_Material_%03i"), p.tex);
						}
					}
				}
			}
		}
	}




#ifdef _DEBUG
	wxLogMessage("M2 Texture Array Built for %s",wxString(m->fullname));
#endif

	// Part Names
	for (unsigned short p=0; p<m->passes.size(); p++) {
		if (m->passes[p].init(m)){
			int g = m->passes[p].geoset;

			wxString partName;
			int mesh = m->geosets[g].id / 100;
			if (m->modelType == MT_CHAR && mesh < 19 && meshes[mesh] != _T("")){
				partName = wxString::Format("Geoset %03i - %s",g,meshes[mesh].c_str());
			}else{
				partName = wxString::Format("Geoset %03i",g);
			}

			partName.Append(_T('\0'));
			if (fmod((float)partName.length(), 2.0f) > 0)
				partName.Append(_T('\0'));
			f.Write(partName.data(), partName.length());
			tagsSize += partName.length();
		}
	}


	if (att!=NULL){
		Model *attM = NULL;
		if (att->model) {
			attM = static_cast<Model*>(att->model);

			if (attM){
				for (size_t i=0; i<attM->passes.size(); i++) {
					ModelRenderPass &p = attM->passes[i];

					if (p.init(attM)) {						
						wxString partName;
						if (att->slot < 15 && slots[att->slot]!=_T("")){
							partName = wxString::Format("%s",slots[att->slot]);
						}else{
							partName = wxString::Format("Slot %02i",att->slot);
						}

						partName.Append(_T('\0'));
						if (fmod((float)partName.length(), 2.0f) > 0)
							partName.Append(_T('\0'));
						f.Write(partName.data(), partName.length());
						tagsSize += partName.length();
					}
				}
			}
		}

		for (size_t i=0; i<att->children.size(); i++) {
			Attachment *att2 = att->children[i];
			for (size_t j=0; j<att2->children.size(); j++) {
				Model *mAttChild = static_cast<Model*>(att2->children[j]->model);

				if (mAttChild){
					for (size_t i=0; i<mAttChild->passes.size(); i++) {
						ModelRenderPass &p = mAttChild->passes[i];

						if (p.init(mAttChild)) {
							int thisslot = att2->children[j]->slot;
							wxString partName;
							if (thisslot < 15 && slots[thisslot]!=_T("")){
								partName = wxString::Format("Child %02i - %s",j,slots[thisslot]);
							}else{
								partName = wxString::Format("Child %02i - Slot %02i",j,att2->children[j]->slot);
							}

							partName.Append(_T('\0'));
							if (fmod((float)partName.length(), 2.0f) > 0)
								partName.Append(_T('\0'));
							f.Write(partName.data(), partName.length());
							tagsSize += partName.length();
						}
					}
				}
			}
		}
	}


	// Surface Name
	//wxString *surfArray = new wxString[m->passes.size()];
	for (unsigned short i=0; i<m->passes.size(); i++) {
		if (m->passes[i].init(m)){
	//		if (!surfArray[m->passes[i].tex]){
				wxString matName;
				matName = texarray[m->passes[i].tex].AfterLast('\\');
				
				if (matName.Len() == 0)
					matName = wxString::Format(_T("Material_%03i"), i);

				matName.Append(_T('\0'));
				if (fmod((float)matName.length(), 2.0f) > 0)
					matName.Append(_T('\0'));
				f.Write(matName.data(), matName.length());
				tagsSize += matName.length();
	//			surfArray[m->passes[i].tex] = texarray[m->passes[i].tex];
	//		}
		}
	}


	if (att!=NULL){
		Model *attM = NULL;
		if (att->model) {
			attM = static_cast<Model*>(att->model);

			if (attM){
				for (size_t i=0; i<attM->passes.size(); i++) {
					ModelRenderPass &p = attM->passes[i];

					if (p.init(attM)) {					
						wxString matName = texarrayAtt[attM->passes[i].tex].AfterLast('\\');

						if (matName.Len() == 0)
							matName = wxString::Format(_T("Attach Material %03i"), i);

						matName.Append(_T('\0'));
						if (fmod((float)matName.length(), 2.0f) > 0)
							matName.Append(_T('\0'));
						f.Write(matName.data(), matName.length());
						tagsSize += matName.length();
					}
				}
			}
		}

		for (size_t a=0; a<att->children.size(); a++) {
			Attachment *att2 = att->children[a];
			for (size_t j=0; j<att2->children.size(); j++) {
				Model *mAttChild = static_cast<Model*>(att2->children[j]->model);

				if (mAttChild){
					for (size_t i=0; i<mAttChild->passes.size(); i++) {
						ModelRenderPass &p = mAttChild->passes[i];

						if (p.init(mAttChild)) {
							int thisslot = att2->children[j]->slot;
							wxString matName;
							if (thisslot < 15 && slots[thisslot]!=_T("")){
								matName = wxString::Format("%s",slots[thisslot]);
							}else{
								matName = texarrayAttc[p.tex].AfterLast('\\');
							}

							if (matName.Len() == 0)
								matName = wxString::Format(_T("Child %02i - Material %03i"), j, i);

							matName.Append(_T('\0'));
							if (fmod((float)matName.length(), 2.0f) > 0)
								matName.Append(_T('\0'));
							f.Write(matName.data(), matName.length());
							tagsSize += matName.length();
						}
					}
				}
			}
		}
	}

	off_t = -4-tagsSize;
	f.SeekO(off_t, wxFromCurrent);
	u32 = reverse_endian<uint32>(tagsSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	f.SeekO(0, wxFromEnd);
	fileLen += tagsSize;
	// ================
#ifdef _DEBUG
	wxLogMessage("M2 Surface Names Written for %s",wxString(m->fullname));
#endif

	

	// ===================================================
	// LAYR
	//
	// Specifies the start of a new layer. Each layer has it's own Point & Poly
	// chunk, which tells it what data is on what layer. It's probably best
	// to only have 1 layer for now.
	// ===================================================
	f.Write("LAYR", 4);
	u32 = reverse_endian<uint32>(18);
	fileLen += 8;
	f.Write(reinterpret_cast<char *>(&u32), 4);
	ub = 0;
	for(int i=0; i<18; i++) {
		f.Write(reinterpret_cast<char *>(&ub), 1);
	}
	fileLen += 18;
	// ================
#ifdef _DEBUG
	wxLogMessage("M2 Layer Defined for %s",wxString(m->fullname));
#endif

	// --
	// POINTS CHUNK, this is the vertice data
	// The PNTS chunk contains triples of floating-point numbers, the coordinates of a list of points. The numbers are written 
	// as IEEE 32-bit floats in network byte order. The IEEE float format is the standard bit pattern used by almost all CPUs 
	// and corresponds to the internal representation of the C language float type. In other words, this isn't some bizarre 
	// proprietary encoding. You can process these using simple fread and fwrite calls (but don't forget to correct the byte 
	// order if necessary).
	uint32 pointsSize = 0;
	f.Write("PNTS", 4);
	u32 = reverse_endian<uint32>(pointsSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	fileLen += 8;

	// output all the model vertice data
	for (size_t i=0; i<m->passes.size(); i++) {
		ModelRenderPass &p = m->passes[i];

		if (p.init(m)) {
			for (size_t k=0, b=p.indexStart; k<p.indexCount; k++,b++) {
				uint16 a = m->indices[b];
				Vec3D vert;
				if ((init == false)&&(m->vertices)) {
					vert.x = reverse_endian<float>(m->vertices[a].x);
					vert.y = reverse_endian<float>(m->vertices[a].y);
					vert.z = reverse_endian<float>(0-m->vertices[a].z);
				} else {
					vert.x = reverse_endian<float>(m->origVertices[a].pos.x);
					vert.y = reverse_endian<float>(m->origVertices[a].pos.y);
					vert.z = reverse_endian<float>(0-m->origVertices[a].pos.z);
				}
				f.Write(reinterpret_cast<char *>(&vert.x), 4);
				f.Write(reinterpret_cast<char *>(&vert.y), 4);
				f.Write(reinterpret_cast<char *>(&vert.z), 4);
				pointsSize += 12;

				numVerts++;
			}
			numGroups++;
		}
	}

	// Output the Attachment vertice data
	if (att!=NULL){
		wxLogMessage("Attachment found! Attempting to save Point Data...");
		Model *attM = NULL;
		if (att->model) {
			attM = static_cast<Model*>(att->model);
			wxLogMessage("Loaded Attached Model %s for export.",attM->modelname);

			if (attM){
				int boneID = -1;
				Model *mParent = NULL;

				if (att->parent) {
					mParent = static_cast<Model*>(att->parent->model);
					if (mParent)
						boneID = mParent->attLookup[att->id];
				}

				Vec3D pos(0,0,0);
				Vec3D scale(1,1,1);
				if (boneID>-1) {
					pos = mParent->atts[boneID].pos;
					Bone cbone = mParent->bones[mParent->atts[boneID].bone];
					Matrix mat = cbone.mat;
					if (init == true){
						// InitPose is a reference to the HandsClosed animation (#15), which is the closest to the Initial pose.
						// By using this animation, we'll get the proper scale for the items when in Init mode.
						int InitPose = 15;
						scale = cbone.scale.getValue(InitPose,0);
						if (scale.x == 0 && scale.y == 0 && scale.z == 0){
							scale.x = 1;
							scale.y = 1;
							scale.z = 1;
						}
					}else{
						scale.x = mat.m[0][0];
						scale.y = mat.m[1][1];
						scale.z = mat.m[2][2];

						mat.translation(cbone.transPivot);

						pos.x = mat.m[0][3];
						pos.y = mat.m[1][3];
						pos.z = mat.m[2][3];
					}
				}

				for (size_t i=0; i<attM->passes.size(); i++) {
					ModelRenderPass &p = attM->passes[i];

					if (p.init(attM)) {
						wxLogMessage("Exporting Point data for Attachment...");
						for (size_t k=0, b=p.indexStart; k<p.indexCount; k++,b++) {
							uint16 a = attM->indices[b];
							Vec3D vert;
							if ((init == false)&&(attM->vertices)) {
								vert.x = reverse_endian<float>((attM->vertices[a].x * scale.x) + pos.x);
								vert.y = reverse_endian<float>((attM->vertices[a].y * scale.y) + pos.y);
								vert.z = reverse_endian<float>(0-(attM->vertices[a].z * scale.z) - pos.z);
							} else {
								vert.x = reverse_endian<float>((attM->origVertices[a].pos.x * scale.x) + pos.x);
								vert.y = reverse_endian<float>((attM->origVertices[a].pos.y * scale.y) + pos.y);
								vert.z = reverse_endian<float>(0-(attM->origVertices[a].pos.z * scale.z) - pos.z);
							}
							f.Write(reinterpret_cast<char *>(&vert.x), 4);
							f.Write(reinterpret_cast<char *>(&vert.y), 4);
							f.Write(reinterpret_cast<char *>(&vert.z), 4);
							pointsSize += 12;
							numVerts++;
						}
						numGroups++;
					}
				}
			}
		}

		for (size_t i=0; i<att->children.size(); i++) {
			Attachment *att2 = att->children[i];
			for (size_t j=0; j<att2->children.size(); j++) {
				Model *mAttChild = static_cast<Model*>(att2->children[j]->model);
				wxLogMessage("Loaded Attached 2nd Child Model %s for export.",mAttChild->fullname);

				if (mAttChild){
					int boneID = -1;
					Model *mParent = NULL;

					if (att2->parent) {
						mParent = static_cast<Model*>(att2->children[j]->parent->model);
						if (mParent)
							boneID = mParent->attLookup[att2->children[j]->id];
					}
					Vec3D pos(0,0,0);
					Vec3D scale(1,1,1);
					if (boneID>-1) {
						pos = mParent->atts[boneID].pos;
						Bone cbone = mParent->bones[mParent->atts[boneID].bone];
						Matrix mat = cbone.mat;
						if (init == true){
							// InitPose is a reference to the HandsClosed animation (#15), which is the closest to the Initial pose.
							// By using this animation, we'll get the proper scale for the items when in Init mode.
							int InitPose = 15;
							scale = cbone.scale.getValue(InitPose,0);
							if (scale.x == 0 && scale.y == 0 && scale.z == 0){
								scale.x = 1;
								scale.y = 1;
								scale.z = 1;
							}
						}else{
							scale.x = mat.m[0][0];
							scale.y = mat.m[1][1];
							scale.z = mat.m[2][2];

							mat.translation(cbone.transPivot);

							pos.x = mat.m[0][3];
							pos.y = mat.m[1][3];
							pos.z = mat.m[2][3];
						}
					}

					for (size_t i=0; i<mAttChild->passes.size(); i++) {
						ModelRenderPass &p = mAttChild->passes[i];

						if (p.init(mAttChild)) {
							wxLogMessage("Exporting Point data for Attached 2nd Child...");
							for (size_t k=0, b=p.indexStart; k<p.indexCount; k++,b++) {
								uint16 a = mAttChild->indices[b];
								Vec3D vert;
								if ((init == false)&&(mAttChild->vertices)) {
									vert.x = reverse_endian<float>((mAttChild->vertices[a].x * scale.x) + pos.x);
									vert.y = reverse_endian<float>((mAttChild->vertices[a].y * scale.y) + pos.y);
									vert.z = reverse_endian<float>(0-(mAttChild->vertices[a].z * scale.z) - pos.z);
								} else {
									vert.x = reverse_endian<float>((mAttChild->origVertices[a].pos.x * scale.x) + pos.x);
									vert.y = reverse_endian<float>((mAttChild->origVertices[a].pos.y * scale.y) + pos.y);
									vert.z = reverse_endian<float>(0-(mAttChild->origVertices[a].pos.z * scale.z) - pos.z);
								}
								f.Write(reinterpret_cast<char *>(&vert.x), 4);
								f.Write(reinterpret_cast<char *>(&vert.y), 4);
								f.Write(reinterpret_cast<char *>(&vert.z), 4);
								pointsSize += 12;
								numVerts++;
							}
							numGroups++;
						}
					}
				}
			}
		}
		wxLogMessage("Finished Attachment Point Data");
	}

	fileLen += pointsSize;
	off_t = -4-pointsSize;
	f.SeekO(off_t, wxFromCurrent);
	u32 = reverse_endian<uint32>(pointsSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	f.SeekO(0, wxFromEnd);
	// ================
#ifdef _DEBUG
	wxLogMessage("M2 Point Data Written for %s",wxString(m->fullname));
#endif

/*
	// --
	// The bounding box for the layer, just so that readers don't have to scan the PNTS chunk to find the extents.
	f.Write("BBOX", 4);
	u32 = reverse_endian<uint32>(24);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	Vec3D vert;
	vert.x = reverse_endian<float>(m->header.ps.BoundingBox[0].x);
	vert.y = reverse_endian<float>(m->header.ps.BoundingBox[0].y);
	vert.z = reverse_endian<float>(m->header.ps.BoundingBox[0].z);
	f.Write(reinterpret_cast<char *>(&vert.x), 4);
	f.Write(reinterpret_cast<char *>(&vert.y), 4);
	f.Write(reinterpret_cast<char *>(&vert.z), 4);
	vert.x = reverse_endian<float>(m->header.ps.BoundingBox[1].x);
	vert.y = reverse_endian<float>(m->header.ps.BoundingBox[1].y);
	vert.z = reverse_endian<float>(m->header.ps.BoundingBox[1].z);
	f.Write(reinterpret_cast<char *>(&vert.x), 4);
	f.Write(reinterpret_cast<char *>(&vert.y), 4);
	f.Write(reinterpret_cast<char *>(&vert.z), 4);
	// ================
*/

	// --
	uint32 vmapSize = 0;


	//Vertex Mapping
	f.Write("VMAP", 4);
	u32 = reverse_endian<uint32>(vmapSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	fileLen += 8;
	// UV Data
	f.Write("TXUV", 4);
	dimension = MSB2(2);
	f.Write(reinterpret_cast<char *>(&dimension), 2);
	f.Write("Texture", 7);
	ub = 0;
	f.Write(reinterpret_cast<char *>(&ub), 1);
	vmapSize += 14;

	counter = 0;

	for (size_t i=0; i<m->passes.size(); i++) {
		ModelRenderPass &p = m->passes[i];

		if (p.init(m)){
			for(int k=0, b=p.indexStart;k<p.indexCount;k++,b++) {
				uint16 a = m->indices[b];

				uint16 indice16;
				uint32 indice32;

				uint16 counter16 = (counter & 0x0000FFFF);
				uint32 counter32 = counter + 0xFF000000;

				if (counter < 0xFF00){
					indice16 = MSB2(counter16);
					f.Write(reinterpret_cast<char *>(&indice16),2);
					vmapSize += 2;
				}else{
					indice32 = reverse_endian<uint32>(counter32);
					f.Write(reinterpret_cast<char *>(&indice32), 4);
					vmapSize += 4;
				}

				f32 = reverse_endian<float>(m->origVertices[a].texcoords.x);
				f.Write(reinterpret_cast<char *>(&f32), 4);
				f32 = reverse_endian<float>(1 - m->origVertices[a].texcoords.y);
				f.Write(reinterpret_cast<char *>(&f32), 4);
				vmapSize += 8;
				counter++;
			}
		}
	}

	if (att!=NULL){
		Model *attM = NULL;
		if (att->model) {
			attM = static_cast<Model*>(att->model);

			if (attM){
				for (size_t i=0; i<attM->passes.size(); i++) {
					ModelRenderPass &p = attM->passes[i];

					if (p.init(attM)) {
						for(int k=0, b=p.indexStart;k<p.indexCount;k++,b++) {
							uint16 a = attM->indices[b];

							uint16 indice16;
							uint32 indice32;

							uint16 counter16 = (counter & 0x0000FFFF);
							uint32 counter32 = counter + 0xFF000000;

							if (counter < 0xFF00){
								indice16 = MSB2(counter16);
								f.Write(reinterpret_cast<char *>(&indice16),2);
								vmapSize += 2;
							}else{
								indice32 = reverse_endian<uint32>(counter32);
								f.Write(reinterpret_cast<char *>(&indice32), 4);
								vmapSize += 4;
							}

							f32 = reverse_endian<float>(attM->origVertices[a].texcoords.x);
							f.Write(reinterpret_cast<char *>(&f32), 4);
							f32 = reverse_endian<float>(1 - attM->origVertices[a].texcoords.y);
							f.Write(reinterpret_cast<char *>(&f32), 4);
							vmapSize += 8;
							counter++;
						}
					}
				}
			}
		}
		for (size_t i=0; i<att->children.size(); i++) {
			Attachment *att2 = att->children[i];
			for (size_t j=0; j<att2->children.size(); j++) {
				Model *mAttChild = static_cast<Model*>(att2->children[j]->model);

				if (mAttChild){
					for (size_t i=0; i<mAttChild->passes.size(); i++) {
						ModelRenderPass &p = mAttChild->passes[i];

						if (p.init(mAttChild)) {
							for(int k=0, b=p.indexStart;k<p.indexCount;k++,b++) {
								uint16 a = mAttChild->indices[b];

								uint16 indice16;
								uint32 indice32;

								uint16 counter16 = (counter & 0x0000FFFF);
								uint32 counter32 = counter + 0xFF000000;

								if (counter < 0xFF00){
									indice16 = MSB2(counter16);
									f.Write(reinterpret_cast<char *>(&indice16),2);
									vmapSize += 2;
								}else{
									indice32 = reverse_endian<uint32>(counter32);
									f.Write(reinterpret_cast<char *>(&indice32), 4);
									vmapSize += 4;
								}

								f32 = reverse_endian<float>(mAttChild->origVertices[a].texcoords.x);
								f.Write(reinterpret_cast<char *>(&f32), 4);
								f32 = reverse_endian<float>(1 - mAttChild->origVertices[a].texcoords.y);
								f.Write(reinterpret_cast<char *>(&f32), 4);
								vmapSize += 8;
								counter++;
							}
						}
					}
				}
			}
		}
	}

	fileLen += vmapSize;

	off_t = -4-vmapSize;
	f.SeekO(off_t, wxFromCurrent);
	u32 = reverse_endian<uint32>(vmapSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	f.SeekO(0, wxFromEnd);


	// ================
#ifdef _DEBUG
	wxLogMessage("M2 VMAP data Written for %s",wxString(m->fullname));
#endif

	// --
	// POLYGON CHUNK
	// The POLS chunk contains a list of polygons. A "polygon" in this context is anything that can be described using an 
	// ordered list of vertices. A POLS of type FACE contains ordinary polygons, but the POLS type can also be CURV, 
	// PTCH, MBAL or BONE, for example.
	//
	// The high 6 bits of the vertex count for each polygon are reserved for flags, which in effect limits the number of 
	// vertices per polygon to 1023. Don't forget to mask the high bits when reading the vertex count. The flags are 
	// currently only defined for CURVs.
	// 
	// The point indexes following the vertex count refer to the points defined in the most recent PNTS chunk. Each index 
	// can be a 2-byte or a 4-byte integer. If the high order (first) byte of the index is not 0xFF, the index is 2 bytes long. 
	// This allows values up to 65279 to be stored in 2 bytes. If the high order byte is 0xFF, the index is 4 bytes long and 
	// its value is in the low three bytes (index & 0x00FFFFFF). The maximum value for 4-byte indexes is 16,777,215 (224 - 1). 
	// Objects with more than 224 vertices can be stored using multiple pairs of PNTS and POLS chunks.
	// 
	// The cube has 6 square faces each defined by 4 vertices. LightWave polygons are single-sided by default 
	// (double-sidedness is a possible surface property). The vertices are listed in clockwise order as viewed from the 
	// visible side, starting with a convex vertex. (The normal is defined as the cross product of the first and last edges.)

	f.Write("POLS", 4);
	uint32 polySize = 4;
	u32 = reverse_endian<uint32>(polySize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	fileLen += 8; // FACE is handled in the PolySize
	f.Write("FACE", 4);

	counter = 0;
	
	for (size_t i=0; i<m->passes.size(); i++) {
		ModelRenderPass &p = m->passes[i];
		if (p.init(m)) {
			for (unsigned int k=0; k<p.indexCount; k+=3) {
				uint16 nverts;

				// Write the number of Verts
				nverts = MSB2(3);
				f.Write(reinterpret_cast<char *>(&nverts),2);
				polySize += 2;

				for (int x=0;x<3;x++,counter++){
					//wxLogMessage("Batch %i, index %i, x=%i",b,k,x);
					uint16 indice16;
					uint32 indice32;

					int mod = 0;
					if (x==1){
						mod = 1;
					}else if (x==2){
						mod = -1;
					}

					uint16 counter16 = ((counter+mod) & 0x0000FFFF);
					uint32 counter32 = (counter+mod) + 0xFF000000;

					if ((counter+mod) < 0xFF00){
						indice16 = MSB2(counter16);
						f.Write(reinterpret_cast<char *>(&indice16),2);
						polySize += 2;
					}else{
						//wxLogMessage("Counter above limit: %i", counter);
						indice32 = reverse_endian<uint32>(counter32);
						f.Write(reinterpret_cast<char *>(&indice32), 4);
						polySize += 4;
					}
				}
			}
		}
	}
	
	if (att!=NULL){
		Model *attM = NULL;
		if (att->model) {
			attM = static_cast<Model*>(att->model);

			if (attM){
				for (size_t i=0; i<attM->passes.size(); i++) {
					ModelRenderPass &p = attM->passes[i];

					if (p.init(attM)) {
						for (unsigned int k=0; k<p.indexCount; k+=3) {
							uint16 nverts;

							// Write the number of Verts
							nverts = MSB2(3);
							f.Write(reinterpret_cast<char *>(&nverts),2);
							polySize += 2;

							for (int x=0;x<3;x++,counter++){
								//wxLogMessage("Batch %i, index %i, x=%i",b,k,x);
								uint16 indice16;
								uint32 indice32;

								int mod = 0;
								if (x==1){
									mod = 1;
								}else if (x==2){
									mod = -1;
								}

								uint16 counter16 = ((counter+mod) & 0x0000FFFF);
								uint32 counter32 = (counter+mod) + 0xFF000000;

								if ((counter+mod) < 0xFF00){
									indice16 = MSB2(counter16);
									f.Write(reinterpret_cast<char *>(&indice16),2);
									polySize += 2;
								}else{
									//wxLogMessage("Counter above limit: %i", counter);
									indice32 = reverse_endian<uint32>(counter32);
									f.Write(reinterpret_cast<char *>(&indice32), 4);
									polySize += 4;
								}
							}
						}
					}
				}
			}
		}
		for (size_t i=0; i<att->children.size(); i++) {
			Attachment *att2 = att->children[i];
			for (size_t j=0; j<att2->children.size(); j++) {
				Model *mAttChild = static_cast<Model*>(att2->children[j]->model);

				if (mAttChild){
					for (size_t i=0; i<mAttChild->passes.size(); i++) {
						ModelRenderPass &p = mAttChild->passes[i];

						if (p.init(mAttChild)) {
							for (unsigned int k=0; k<p.indexCount; k+=3) {
								uint16 nverts;

								// Write the number of Verts
								nverts = MSB2(3);
								f.Write(reinterpret_cast<char *>(&nverts),2);
								polySize += 2;

								for (int x=0;x<3;x++,counter++){
									//wxLogMessage("Batch %i, index %i, x=%i",b,k,x);
									uint16 indice16;
									uint32 indice32;

									int mod = 0;
									if (x==1){
										mod = 1;
									}else if (x==2){
										mod = -1;
									}

									uint16 counter16 = ((counter+mod) & 0x0000FFFF);
									uint32 counter32 = (counter+mod) + 0xFF000000;

									if ((counter+mod) < 0xFF00){
										indice16 = MSB2(counter16);
										f.Write(reinterpret_cast<char *>(&indice16),2);
										polySize += 2;
									}else{
										//wxLogMessage("Counter above limit: %i", counter);
										indice32 = reverse_endian<uint32>(counter32);
										f.Write(reinterpret_cast<char *>(&indice32), 4);
										polySize += 4;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	off_t = -4-polySize;
	f.SeekO(off_t, wxFromCurrent);
	u32 = reverse_endian<uint32>(polySize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	f.SeekO(0, wxFromEnd);

	fileLen += polySize;
	// ========
#ifdef _DEBUG
	wxLogMessage("M2 Polygons Written for %s",wxString(m->fullname));
#endif

	// The PTAG chunk associates tags with polygons. In this case, it identifies which surface is assigned to each polygon. 
	// The first number in each pair is a 0-based index into the most recent POLS chunk, and the second is a 0-based 
	// index into the TAGS chunk.

	// NOTE: Every PTAG type needs a seperate PTAG call!

	TagCounter = 0;
	PartCounter = 0;
	SurfCounter = 0;
	counter = 0;
	int32 ptagSize;

	// Parts PolyTag
	f.Write("PTAG", 4);
	ptagSize = 4;
	counter=0;
	u32 = reverse_endian<uint32>(ptagSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	fileLen += 8;
	f.Write("PART", 4);
	for (int i=0;i<m->passes.size();i++) {
		ModelRenderPass &p = m->passes[i];

		if (p.init(m)){
			for (unsigned int k=0; k<p.indexCount; k+=3) {
				uint16 indice16;
				uint32 indice32;

				uint16 counter16 = (counter & 0x0000FFFF);
				uint32 counter32 = counter + 0xFF000000;

				if (counter < 0xFF00){
					indice16 = MSB2(counter16);
					f.Write(reinterpret_cast<char *>(&indice16),2);
					ptagSize += 2;
				}else{
					indice32 = reverse_endian<uint32>(counter32);
					f.Write(reinterpret_cast<char *>(&indice32), 4);
					ptagSize += 4;
				}

				u16 = MSB2(TagCounter);
				f.Write(reinterpret_cast<char *>(&u16), 2);
				ptagSize += 2;
				counter++;
			}
			TagCounter++;
			PartCounter++;
		}
	}
	
	if (att!=NULL){
		Model *attM = NULL;
		if (att->model) {
			attM = static_cast<Model*>(att->model);

			if (attM){
				for (size_t i=0; i<attM->passes.size(); i++) {
					ModelRenderPass &p = attM->passes[i];

					if (p.init(attM)) {
						for (unsigned int k=0; k<p.indexCount; k+=3) {
							uint16 indice16;
							uint32 indice32;

							uint16 counter16 = (counter & 0x0000FFFF);
							uint32 counter32 = counter + 0xFF000000;

							if (counter < 0xFF00){
								indice16 = MSB2(counter16);
								f.Write(reinterpret_cast<char *>(&indice16),2);
								ptagSize += 2;
							}else{
								indice32 = reverse_endian<uint32>(counter32);
								f.Write(reinterpret_cast<char *>(&indice32), 4);
								ptagSize += 4;
							}

							u16 = MSB2(TagCounter);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							ptagSize += 2;
							counter++;
						}
						TagCounter++;
						PartCounter++;
					}
				}
			}
		}
		for (size_t i=0; i<att->children.size(); i++) {
			Attachment *att2 = att->children[i];
			for (size_t j=0; j<att2->children.size(); j++) {
				Model *mAttChild = static_cast<Model*>(att2->children[j]->model);

				if (mAttChild){
					for (size_t i=0; i<mAttChild->passes.size(); i++) {
						ModelRenderPass &p = mAttChild->passes[i];

						if (p.init(mAttChild)) {
							for (unsigned int k=0; k<p.indexCount; k+=3) {
								uint16 indice16;
								uint32 indice32;

								uint16 counter16 = (counter & 0x0000FFFF);
								uint32 counter32 = counter + 0xFF000000;

								if (counter < 0xFF00){
									indice16 = MSB2(counter16);
									f.Write(reinterpret_cast<char *>(&indice16),2);
									ptagSize += 2;
								}else{
									indice32 = reverse_endian<uint32>(counter32);
									f.Write(reinterpret_cast<char *>(&indice32), 4);
									ptagSize += 4;
								}

								u16 = MSB2(TagCounter);
								f.Write(reinterpret_cast<char *>(&u16), 2);
								ptagSize += 2;
								counter++;
							}
							TagCounter++;
							PartCounter++;
						}
					}
				}
			}
		}
	}


	fileLen += ptagSize;

	off_t = -4-ptagSize;
	f.SeekO(off_t, wxFromCurrent);
	u32 = reverse_endian<uint32>(ptagSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	f.SeekO(0, wxFromEnd);


	// Surface PolyTag
	counter=0;
	ptagSize = 4;
	f.Write("PTAG", 4);
	u32 = reverse_endian<uint32>(ptagSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	fileLen += 8;
	f.Write("SURF", 4);
	for (size_t i=0; i<m->passes.size(); i++) {
		ModelRenderPass &p = m->passes[i];

		if (p.init(m)) {
			for (unsigned int k=0; k<p.indexCount; k+=3) {
				uint16 indice16;
				uint32 indice32;

				uint16 counter16 = (counter & 0x0000FFFF);
				uint32 counter32 = counter + 0xFF000000;

				if (counter < 0xFF00){
					indice16 = MSB2(counter16);
					f.Write(reinterpret_cast<char *>(&indice16),2);
					ptagSize += 2;
				}else{
					indice32 = reverse_endian<uint32>(counter32);
					f.Write(reinterpret_cast<char *>(&indice32), 4);
					ptagSize += 4;
				}

				u16 = MSB2(TagCounter);
				f.Write(reinterpret_cast<char *>(&u16), 2);
				ptagSize += 2;
				counter++;
			}
			TagCounter++;
			SurfCounter++;
		}
	}

	if (att!=NULL){
		Model *attM = NULL;
		if (att->model) {
			attM = static_cast<Model*>(att->model);

			if (attM){
				for (size_t i=0; i<attM->passes.size(); i++) {
					ModelRenderPass &p = attM->passes[i];

					if (p.init(attM)) {
						for (unsigned int k=0; k<p.indexCount; k+=3) {
							uint16 indice16;
							uint32 indice32;

							uint16 counter16 = (counter & 0x0000FFFF);
							uint32 counter32 = counter + 0xFF000000;

							if (counter < 0xFF00){
								indice16 = MSB2(counter16);
								f.Write(reinterpret_cast<char *>(&indice16),2);
								ptagSize += 2;
							}else{
								indice32 = reverse_endian<uint32>(counter32);
								f.Write(reinterpret_cast<char *>(&indice32), 4);
								ptagSize += 4;
							}

							u16 = MSB2(TagCounter);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							ptagSize += 2;
							counter++;
						}
						TagCounter++;
						SurfCounter++;
					}
				}
			}
		}
		for (size_t i=0; i<att->children.size(); i++) {
			Attachment *att2 = att->children[i];
			for (size_t j=0; j<att2->children.size(); j++) {
				Model *mAttChild = static_cast<Model*>(att2->children[j]->model);

				if (mAttChild){
					for (size_t i=0; i<mAttChild->passes.size(); i++) {
						ModelRenderPass &p = mAttChild->passes[i];

						if (p.init(mAttChild)) {
							for (unsigned int k=0; k<p.indexCount; k+=3) {
								uint16 indice16;
								uint32 indice32;

								uint16 counter16 = (counter & 0x0000FFFF);
								uint32 counter32 = counter + 0xFF000000;

								if (counter < 0xFF00){
									indice16 = MSB2(counter16);
									f.Write(reinterpret_cast<char *>(&indice16),2);
									ptagSize += 2;
								}else{
									indice32 = reverse_endian<uint32>(counter32);
									f.Write(reinterpret_cast<char *>(&indice32), 4);
									ptagSize += 4;
								}

								u16 = MSB2(TagCounter);
								f.Write(reinterpret_cast<char *>(&u16), 2);
								ptagSize += 2;
								counter++;
							}
							TagCounter++;
							SurfCounter++;
						}
					}
				}
			}
		}
	}
	
	fileLen += ptagSize;

	off_t = -4-ptagSize;
	f.SeekO(off_t, wxFromCurrent);
	u32 = reverse_endian<uint32>(ptagSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	f.SeekO(0, wxFromEnd);


	// ================
#ifdef _DEBUG
	wxLogMessage("M2 PTag Surface data Written for %s",wxString(m->fullname));
#endif

	// --
	int32 vmadSize = 0;
	f.Write("VMAD", 4);
	u32 = reverse_endian<uint32>(vmadSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	fileLen += 8;
	f.Write("TXUV", 4);
	dimension = 2;
	dimension = MSB2(dimension);
	f.Write(reinterpret_cast<char *>(&dimension), 2);
	f.Write("Texture", 7);
	ub = 0;
	f.Write(reinterpret_cast<char *>(&ub), 1);
	vmadSize += 14;

	counter = 0;
	for (size_t i=0; i<m->passes.size(); i++) {
		ModelRenderPass &p = m->passes[i];

		if (p.init(m)) {
			for (size_t k=0, b=p.indexStart; k<p.indexCount; k++,b++) {
				int a = m->indices[b];
				uint16 indice16;
				uint32 indice32;

				uint16 counter16 = (counter & 0x0000FFFF);
				uint32 counter32 = counter + 0xFF000000;

				if (counter < 0xFF00){
					indice16 = MSB2(counter16);
					f.Write(reinterpret_cast<char *>(&indice16),2);
					vmadSize += 2;
				}else{
					indice32 = reverse_endian<uint32>(counter32);
					f.Write(reinterpret_cast<char *>(&indice32), 4);
					vmadSize += 4;
				}

				if (((counter/3) & 0x0000FFFF) < 0xFF00){
					indice16 = MSB2(((counter/3) & 0x0000FFFF));
					f.Write(reinterpret_cast<char *>(&indice16),2);
					vmadSize += 2;
				}else{
					indice32 = reverse_endian<uint32>((counter/3) + 0xFF000000);
					f.Write(reinterpret_cast<char *>(&indice32), 4);
					vmadSize += 4;
				}

				f32 = reverse_endian<float>(m->origVertices[a].texcoords.x);
				f.Write(reinterpret_cast<char *>(&f32), 4);
				f32 = reverse_endian<float>(1 - m->origVertices[a].texcoords.y);
				f.Write(reinterpret_cast<char *>(&f32), 4);
				counter++;
				vmadSize += 8;
			}
		}
	}
	
	if (att!=NULL){
		Model *attM = NULL;
		if (att->model) {
			attM = static_cast<Model*>(att->model);

			if (attM){
				for (size_t i=0; i<attM->passes.size(); i++) {
					ModelRenderPass &p = attM->passes[i];

					if (p.init(attM)) {
						for (size_t k=0, b=p.indexStart; k<p.indexCount; k++,b++) {
							uint16 a = attM->indices[b];
							uint16 indice16;
							uint32 indice32;

							uint16 counter16 = (counter & 0x0000FFFF);
							uint32 counter32 = counter + 0xFF000000;

							if (counter < 0xFF00){
								indice16 = MSB2(counter16);
								f.Write(reinterpret_cast<char *>(&indice16),2);
								vmadSize += 2;
							}else{
								indice32 = reverse_endian<uint32>(counter32);
								f.Write(reinterpret_cast<char *>(&indice32), 4);
								vmadSize += 4;
							}

							if (((counter/3) & 0x0000FFFF) < 0xFF00){
								indice16 = MSB2(((counter/3) & 0x0000FFFF));
								f.Write(reinterpret_cast<char *>(&indice16),2);
								vmadSize += 2;
							}else{
								indice32 = reverse_endian<uint32>((counter/3) + 0xFF000000);
								f.Write(reinterpret_cast<char *>(&indice32), 4);
								vmadSize += 4;
							}
							f32 = reverse_endian<float>(attM->origVertices[a].texcoords.x);
							f.Write(reinterpret_cast<char *>(&f32), 4);
							f32 = reverse_endian<float>(1 - attM->origVertices[a].texcoords.y);
							f.Write(reinterpret_cast<char *>(&f32), 4);
							counter++;
							vmadSize += 8;
						}
					}
				}
			}
		}
		for (size_t i=0; i<att->children.size(); i++) {
			Attachment *att2 = att->children[i];
			for (size_t j=0; j<att2->children.size(); j++) {
				Model *mAttChild = static_cast<Model*>(att2->children[j]->model);

				if (mAttChild){
					for (size_t i=0; i<mAttChild->passes.size(); i++) {
						ModelRenderPass &p = mAttChild->passes[i];

						if (p.init(mAttChild)) {
							for (size_t k=0, b=p.indexStart; k<p.indexCount; k++,b++) {
								uint16 a = mAttChild->indices[b];
								uint16 indice16;
								uint32 indice32;

								uint16 counter16 = (counter & 0x0000FFFF);
								uint32 counter32 = counter + 0xFF000000;

								if (counter < 0xFF00){
									indice16 = MSB2(counter16);
									f.Write(reinterpret_cast<char *>(&indice16),2);
									vmadSize += 2;
								}else{
									indice32 = reverse_endian<uint32>(counter32);
									f.Write(reinterpret_cast<char *>(&indice32), 4);
									vmadSize += 4;
								}

								if (((counter/3) & 0x0000FFFF) < 0xFF00){
									indice16 = MSB2(((counter/3) & 0x0000FFFF));
									f.Write(reinterpret_cast<char *>(&indice16),2);
									vmadSize += 2;
								}else{
									indice32 = reverse_endian<uint32>((counter/3) + 0xFF000000);
									f.Write(reinterpret_cast<char *>(&indice32), 4);
									vmadSize += 4;
								}
								f32 = reverse_endian<float>(mAttChild->origVertices[a].texcoords.x);
								f.Write(reinterpret_cast<char *>(&f32), 4);
								f32 = reverse_endian<float>(1 - mAttChild->origVertices[a].texcoords.y);
								f.Write(reinterpret_cast<char *>(&f32), 4);
								counter++;
								vmadSize += 8;
							}
						}
					}
				}
			}
		}
	}

	fileLen += vmadSize;
	off_t = -4-vmadSize;
	f.SeekO(off_t, wxFromCurrent);
	u32 = reverse_endian<uint32>(vmadSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	f.SeekO(0, wxFromEnd);
	// ================
#ifdef _DEBUG
	wxLogMessage("M2 VMAD data Written for %s",wxString(m->fullname));
#endif

	// --
	uint32 surfaceCounter = PartCounter;
	
	for (size_t i=0; i<m->passes.size(); i++) {
		ModelRenderPass &p = m->passes[i];

		if (p.init(m)) {
			int clipSize = 0;
			f.Write("CLIP", 4);
			u32 = reverse_endian<uint32>(0);
			f.Write(reinterpret_cast<char *>(&u32), 4);
			fileLen += 8;

			u32 = reverse_endian<uint32>(++surfaceCounter);
			f.Write(reinterpret_cast<char *>(&u32), 4);
			f.Write("STIL", 4);
			clipSize += 8;

#ifdef _DEBUG
			if (p.useTex2 != false){
				wxLogMessage("Pass %i uses Texture 2!",i);
			}
#endif

			wxString FilePath = wxString(fn, wxConvUTF8).BeforeLast('\\');
			wxString texName = texarray[p.tex];
			wxString texPath = texName.BeforeLast('\\');
			if (m->modelType == MT_CHAR){
				texName = wxString(fn, wxConvUTF8).AfterLast('\\').BeforeLast('.') + "_" + texName.AfterLast('\\');
			}else if ((texName.Find('\\') <= 0)&&(texName == "Cape")){
				texName = wxString(fn, wxConvUTF8).AfterLast('\\').BeforeLast('.') + "_Replacable";
				texPath = wxString(m->name.c_str()).BeforeLast('\\');
			}else if (texName.Find('\\') <= 0){
				texName = wxString(fn, wxConvUTF8).AfterLast('\\').BeforeLast('.') + "_" + texName;
				texPath = wxString(m->name.c_str()).BeforeLast('\\');
			}else{
				texName = texName.AfterLast('\\');
			}

			if (texName.Length() == 0)
				texName << wxString(m->modelname.c_str()).AfterLast('\\').BeforeLast('.') << wxString::Format("_Image_%03i",i);

			wxString sTexName = "";
			if (modelExport_LW_PreserveDir == true){
				sTexName += "Images/";
			}
			if (m->modelType != MT_CHAR){
				if (modelExport_PreserveDir == true){
					sTexName += texPath;
					sTexName << "\\";
					sTexName.Replace("\\","/");
				}
			}
			sTexName += texName;

			sTexName << _T(".tga") << '\0';

			if (fmod((float)sTexName.length(), 2.0f) > 0)
				sTexName.Append(_T('\0'));

			u16 = MSB2(sTexName.length());
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f.Write(sTexName.data(), sTexName.length());
			clipSize += (2+sTexName.length());

			// update the chunks length
			off_t = -4-clipSize;
			f.SeekO(off_t, wxFromCurrent);
			u32 = reverse_endian<uint32>(clipSize);
			f.Write(reinterpret_cast<char *>(&u32), 4);
			f.SeekO(0, wxFromEnd);

			// save texture to file
			wxString texFilename(fn, wxConvUTF8);
			texFilename = texFilename.BeforeLast('\\');
			texFilename += '\\';
			texFilename += texName;

			if (modelExport_LW_PreserveDir == true){
				wxString Path, Name;

				Path << wxString(fn, wxConvUTF8).BeforeLast('\\');
				Name << texFilename.AfterLast('\\');

				MakeDirs(Path,"Images");

				texFilename.Empty();
				texFilename << Path << "\\Images\\" << Name;
			}
			if (m->modelType != MT_CHAR){
				if (modelExport_PreserveDir == true){
					wxString Path1, Path2, Name;
					Path1 << texFilename.BeforeLast('\\');
					Name << texName.AfterLast('\\');
					Path2 << texPath;

					MakeDirs(Path1,Path2);

					texFilename.Empty();
					texFilename << Path1 << '\\' << Path2 << '\\' << Name;
				}
			}
			if (texFilename.Length() == 0){
				texFilename << wxString(m->modelname.c_str()).AfterLast('\\').BeforeLast('.') << wxString::Format("_Image_%03i",i);
			}
			texFilename << (".tga");
#ifdef _DEBUG
			wxLogMessage("Image Export: Final texFilename result: %s", texFilename);
#endif
			SaveTexture(texFilename);

			fileLen += clipSize;
		}
	}

	if (att!=NULL){
		Model *attM = NULL;
		if (att->model) {
			attM = static_cast<Model*>(att->model);

			if (attM){
				for (size_t i=0; i<attM->passes.size(); i++) {
					ModelRenderPass &p = attM->passes[i];

					if (p.init(attM)) {
						int clipSize = 0;
						f.Write("CLIP", 4);
						u32 = reverse_endian<uint32>(0);
						f.Write(reinterpret_cast<char *>(&u32), 4);
						fileLen += 8;

						u32 = reverse_endian<uint32>(++surfaceCounter);
						f.Write(reinterpret_cast<char *>(&u32), 4);
						f.Write("STIL", 4);
						clipSize += 8;

						wxString FilePath = wxString(fn, wxConvUTF8).BeforeLast('\\');
						wxString texName = texarrayAtt[p.tex];
						wxString texPath = texName.BeforeLast('\\');
						if (texName.AfterLast('\\') == "Cape"){
							texName = wxString(fn, wxConvUTF8).AfterLast('\\').BeforeLast('.') + wxString(attM->name.c_str()).AfterLast('\\').BeforeLast('.') + "_Replacable";
							texPath = wxString(fn, wxConvUTF8).BeforeLast('\\');
						}else{
							texName = texName.AfterLast('\\');
						}

						if (texName.Length() == 0){
							texName << wxString(attM->modelname.c_str()).AfterLast('\\').BeforeLast('.');
							texPath = wxString(attM->name.c_str()).BeforeLast('\\');
						}

/*
						//texName = attM->TextureList[p.tex];

						if ((texName.Find('\\') <= 0)&&(texName == "Cape")){
							texName = wxString(fn, wxConvUTF8).AfterLast('\\').BeforeLast('.') + "_Replacable";
							texPath = wxString(attM->name).BeforeLast('\\');
						}else{
							texName = texName.AfterLast('\\');
						}

						if (texName.Length() == 0)
							texName << wxString(attM->modelname).AfterLast('\\').BeforeLast('.') << wxString::Format("_Image_%03i",i);
*/
						wxString sTexName = "";
						if (modelExport_LW_PreserveDir == true){
							sTexName += "Images/";
						}
						if (modelExport_PreserveDir == true){
							sTexName += texPath;
							sTexName << "\\";
							sTexName.Replace("\\","/");
						}
						sTexName += texName;

						sTexName << _T(".tga") << '\0';

						if (fmod((float)sTexName.length(), 2.0f) > 0)
							sTexName.Append(_T('\0'));

						u16 = MSB2(sTexName.length());
						f.Write(reinterpret_cast<char *>(&u16), 2);
						f.Write(sTexName.data(), sTexName.length());
						clipSize += (2+sTexName.length());

						// update the chunks length
						off_t = -4-clipSize;
						f.SeekO(off_t, wxFromCurrent);
						u32 = reverse_endian<uint32>(clipSize);
						f.Write(reinterpret_cast<char *>(&u32), 4);
						f.SeekO(0, wxFromEnd);

						// save texture to file
						wxString texFilename(fn, wxConvUTF8);
						texFilename = texFilename.BeforeLast('\\');
						texFilename += '\\';
						texFilename += texName;

						if (modelExport_LW_PreserveDir == true){
							wxString Path, Name;

							Path << wxString(fn, wxConvUTF8).BeforeLast('\\');
							Name << texFilename.AfterLast('\\');

							MakeDirs(Path,"Images");

							texFilename.Empty();
							texFilename << Path << "\\Images\\" << Name;
						}
						if (modelExport_PreserveDir == true){
							wxString Path1, Path2, Name;
							Path1 << texFilename.BeforeLast('\\');
							Name << texName.AfterLast('\\');
							Path2 << texPath;

							MakeDirs(Path1,Path2);

							texFilename.Empty();
							texFilename << Path1 << '\\' << Path2 << '\\' << Name;
						}
						
						if (texFilename.Length() == 0){
							texFilename << wxString(attM->modelname.c_str()).AfterLast('\\').BeforeLast('.') << wxString::Format("_Image_%03i",i);
						}
						texFilename << (".tga");
						SaveTexture(texFilename);

						fileLen += clipSize;
					}
				}
			}
		}
		for (size_t i=0; i<att->children.size(); i++) {
			Attachment *att2 = att->children[i];
			for (size_t j=0; j<att2->children.size(); j++) {
				Model *mAttChild = static_cast<Model*>(att2->children[j]->model);

				if (mAttChild){
					for (size_t k=0; k<mAttChild->passes.size(); k++) {
						ModelRenderPass &p = mAttChild->passes[k];

						if (p.init(mAttChild)) {
							int clipSize = 0;
							f.Write("CLIP", 4);
							u32 = reverse_endian<uint32>(0);
							f.Write(reinterpret_cast<char *>(&u32), 4);
							fileLen += 8;

							u32 = reverse_endian<uint32>(++surfaceCounter);
							f.Write(reinterpret_cast<char *>(&u32), 4);
							f.Write("STIL", 4);
							clipSize += 8;

							wxString FilePath = wxString(fn, wxConvUTF8).BeforeLast('\\');
							wxString texName = texarrayAttc[p.tex];
							wxString texPath = texName.BeforeLast('\\');
							if (texName.AfterLast('\\') == "Cape"){
								//texName = wxString(fn, wxConvUTF8).AfterLast('\\').BeforeLast('.') + wxString(mAttChild->name).AfterLast('\\').BeforeLast('.') + "_Replacable";
								texName = wxString(mAttChild->name.c_str()).AfterLast('\\').BeforeLast('.');
								texPath = wxString(mAttChild->name.c_str()).BeforeLast('\\');
							}else{
								texName = texName.AfterLast('\\');
							}

							if (texName.Length() == 0){
								texName << wxString(mAttChild->modelname.c_str()).AfterLast('\\').BeforeLast('.');
								texPath = wxString(mAttChild->name.c_str()).BeforeLast('\\');
							}

							wxString sTexName = "";
							if (modelExport_LW_PreserveDir == true){
								sTexName += "Images/";
							}
							if (modelExport_PreserveDir == true){
								sTexName += texPath;
								sTexName << "\\";
								sTexName.Replace("\\","/");
							}
							sTexName += texName;

							sTexName << _T(".tga") << '\0';

							if (fmod((float)sTexName.length(), 2.0f) > 0)
								sTexName.Append(_T('\0'));

							u16 = MSB2(sTexName.length());
							f.Write(reinterpret_cast<char *>(&u16), 2);
							f.Write(sTexName.data(), sTexName.length());
							clipSize += (2+sTexName.length());

							// update the chunks length
							off_t = -4-clipSize;
							f.SeekO(off_t, wxFromCurrent);
							u32 = reverse_endian<uint32>(clipSize);
							f.Write(reinterpret_cast<char *>(&u32), 4);
							f.SeekO(0, wxFromEnd);

							// save texture to file
							wxString texFilename(fn, wxConvUTF8);
							texFilename = texFilename.BeforeLast('\\');
							texFilename += '\\';
							texFilename += texName;

							if (modelExport_LW_PreserveDir == true){
								wxString Path, Name;

								Path << wxString(fn, wxConvUTF8).BeforeLast('\\');
								Name << texFilename.AfterLast('\\');

								MakeDirs(Path,"Images");

								texFilename.Empty();
								texFilename << Path << "\\Images\\" << Name;
							}
							if (modelExport_PreserveDir == true){
								wxString Path1, Path2, Name;
								Path1 << texFilename.BeforeLast('\\');
								Name << texName.AfterLast('\\');
								Path2 << texPath;

								MakeDirs(Path1,Path2);

								texFilename.Empty();
								texFilename << Path1 << '\\' << Path2 << '\\' << Name;
							}
							
							if (texFilename.Length() == 0){
								texFilename << wxString(attM->modelname.c_str()).AfterLast('\\').BeforeLast('.') << wxString::Format("_Image_%03i",i);
							}
							texFilename << (".tga");
							SaveTexture(texFilename);

							fileLen += clipSize;
						}
					}
				}
			}
		}
	}

	// ================
#ifdef _DEBUG
	wxLogMessage("M2 Images & Image Data Written for %s",wxString(m->fullname));
#endif

	// --
	wxString surfName;
	surfaceCounter = PartCounter;
	for (size_t i=0; i<m->passes.size(); i++) {
		ModelRenderPass &p = m->passes[i];

		if (p.init(m)) {
			uint32 surfaceDefSize = 0;
			f.Write("SURF", 4);
			u32 = reverse_endian<uint32>(surfaceDefSize);
			f.Write(reinterpret_cast<char *>(&u32), 4);
			fileLen += 8;

			// Surface name
			surfName = texarray[p.tex].AfterLast('\\');
			if (surfName.Len() == 0)
				surfName = wxString::Format(_T("Material_%03i"),p.tex);
			
			surfaceCounter++;
			
			surfName.Append(_T('\0'));
			if (fmod((float)surfName.length(), 2.0f) > 0)
				surfName.Append(_T('\0'));

			surfName.Append(_T('\0')); // ""
			surfName.Append(_T('\0'));
			f.Write(surfName.data(), (int)surfName.length());
			
			surfaceDefSize += (uint32)surfName.length();

			// Surface Attributes
			// COLOUR, size 4, bytes 2
			f.Write("COLR", 4);
			u16 = 14; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			
			// value
			f32 = reverse_endian<float>(p.ocol.x);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f32 = reverse_endian<float>(p.ocol.y);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f32 = reverse_endian<float>(p.ocol.z);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			
			surfaceDefSize += 20;

			// LUMI
			f.Write("LUMI", 4);
			u16 = 6; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 0;
			f32 = reverse_endian<float>(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);


			surfaceDefSize += 12;

			// DIFF
			f.Write("DIFF", 4);
			u16 = 6; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 1;
			f32 = reverse_endian<float>(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);

			surfaceDefSize += 12;


			// SPEC
			f.Write("SPEC", 4);
			u16 = 6; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 0;
			f32 = reverse_endian<float>(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);

			surfaceDefSize += 12;

			// REFL
			f.Write("REFL", 4);
			u16 = 6; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			ub = '\0';
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);

			surfaceDefSize += 12;

			// TRAN
			f.Write("TRAN", 4);
			u16 = 6; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			ub = '\0';
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);

			surfaceDefSize += 12;

			// GLOSSINESS
			f.Write("GLOS", 4);
			u16 = 6; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			// Value
			// Set to 20%, because that seems right.
			f32 = 0.2;
			f32 = reverse_endian<float>(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			surfaceDefSize += 12;

			// SMAN (Smoothing)
			f.Write("SMAN", 4);
			u16 = 4; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			// Smoothing is done in radiens. PI = 180 degree smoothing.
			f32 = PI;
			f32 = reverse_endian<float>(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			surfaceDefSize += 10;

			// RFOP
			f.Write("RFOP", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 1;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);

			surfaceDefSize += 8;

			// TROP
			f.Write("TROP", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 1;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);

			surfaceDefSize += 8;

			// SIDE
			f.Write("SIDE", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 1;
			// If double-sided...
			if (p.cull == false){
				u16 = 3;
			}
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);

			surfaceDefSize += 8;

			
			// --
			// BLOK
			uint16 blokSize = 0;
			f.Write("BLOK", 4);
			f.Write(reinterpret_cast<char *>(&blokSize), 2);
			surfaceDefSize += 6;

			// IMAP
			f.Write("IMAP", 4);
			u16 = 50-8; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 0x80;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;

			// CHAN
			f.Write("CHAN", 4);
			u16 = 4; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f.Write("COLR", 4);
			blokSize += 10;

			// OPAC
			f.Write("OPAC", 4);
			u16 = 8; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 1.0;
			f32 = reverse_endian<float>(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 14;

			// ENAB
			f.Write("ENAB", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 1;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;

			// NEGA
			f.Write("NEGA", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 0;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;
/*
			// AXIS
			f.Write("AXIS", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 1;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;
*/
			// TMAP
			f.Write("TMAP", 4);
			u16 = 98; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 6;

			// CNTR
			f.Write("CNTR", 4);
			u16 = 14; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 0.0;
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 20;

			// SIZE
			f.Write("SIZE", 4);
			u16 = 14; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 1.0;
			f32 = reverse_endian<float>(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 20;

			// ROTA
			f.Write("ROTA", 4);
			u16 = 14; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 0.0;
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 20;

			// FALL
			f.Write("FALL", 4);
			u16 = 16; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 0.0;
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 22;

			// OREF
			f.Write("OREF", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;

			// CSYS
			f.Write("CSYS", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;

			// end TMAP

			// PROJ
			f.Write("PROJ", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 5;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;

			// AXIS
			f.Write("AXIS", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 2;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;

			// IMAG
			f.Write("IMAG", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = surfaceCounter;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;

			// WRAP
			f.Write("WRAP", 4);
			u16 = 4; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 1;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 1;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 10;

			// WRPW
			f.Write("WRPW", 4);
			u16 = 6; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 1;
			f32 = reverse_endian<float>(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 12;

			// WRPH
			f.Write("WRPH", 4);
			u16 = 6; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 1;
			f32 = reverse_endian<float>(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 12;

			// VMAP
			f.Write("VMAP", 4);
			u16 = 8; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			wxString t = _T("Texture");
			t.Append(_T('\0'));
			f.Write(t.data(), t.length());
			blokSize += 14;

			// AAST
			f.Write("AAST", 4);
			u16 = 6; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 1;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 1;
			f32 = reverse_endian<float>(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			blokSize += 12;

			// PIXB
			f.Write("PIXB", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 1;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;

			// Fix Blok Size
			surfaceDefSize += blokSize;
			off_t = -2-blokSize;
			f.SeekO(off_t, wxFromCurrent);
			u16 = MSB2(blokSize);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f.SeekO(0, wxFromEnd);
			// ================
			

			// CMNT
			f.Write("CMNT", 4);
			wxString cmnt;
			if (m->modelType == MT_CHAR){
				cmnt = wxString::Format(_T("Character Material %03i"),p.tex);
			}else{
				cmnt = texarray[p.tex];
			}
			cmnt.Append(_T('\0'));
			if (fmod((float)cmnt.length(), 2.0f) > 0)
				cmnt.Append(_T('\0'));
			u16 = cmnt.length(); // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f.Write(cmnt.data(), cmnt.length());
			surfaceDefSize += 6 + cmnt.length();

			f.Write("VERS", 4);
			u16 = 4;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 950;
			f32 = reverse_endian<int32>(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			surfaceDefSize += 10;
			
			// Fix Surface Size
			fileLen += surfaceDefSize;
			off_t = -4-surfaceDefSize;
			f.SeekO(off_t, wxFromCurrent);
			u32 = reverse_endian<uint32>(surfaceDefSize);
			f.Write(reinterpret_cast<char *>(&u32), 4);
			f.SeekO(0, wxFromEnd);
		}
	}

	if (att!=NULL){
		Model *attM = NULL;
		if (att->model) {
			attM = static_cast<Model*>(att->model);

			if (attM){
				for (size_t i=0; i<attM->passes.size(); i++) {
					ModelRenderPass &p = attM->passes[i];

					if (p.init(attM)) {
						uint32 surfaceDefSize = 0;
						f.Write("SURF", 4);
						u32 = reverse_endian<uint32>(surfaceDefSize);
						f.Write(reinterpret_cast<char *>(&u32), 4);
						fileLen += 8;

						// Surface name
						wxString surfName = texarrayAtt[attM->passes[i].tex].AfterLast('\\');

						if (surfName.Len() == 0)
							surfName = wxString::Format(_T("Attach Material %03i"), i);

						surfaceCounter++;
						
						surfName.Append(_T('\0'));
						if (fmod((float)surfName.length(), 2.0f) > 0)
							surfName.Append(_T('\0'));

						surfName.Append(_T('\0')); // ""
						surfName.Append(_T('\0'));
						f.Write(surfName.data(), (int)surfName.length());
						
						surfaceDefSize += (uint32)surfName.length();

						// Surface Attributes
						// COLOUR, size 4, bytes 2
						f.Write("COLR", 4);
						u16 = 14; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						
						// value
						f32 = reverse_endian<float>(p.ocol.x);
						f.Write(reinterpret_cast<char *>(&f32), 4);
						f32 = reverse_endian<float>(p.ocol.y);
						f.Write(reinterpret_cast<char *>(&f32), 4);
						f32 = reverse_endian<float>(p.ocol.z);
						f.Write(reinterpret_cast<char *>(&f32), 4);
						u16 = 0;
						f.Write(reinterpret_cast<char *>(&u16), 2);
						
						surfaceDefSize += 20;

						// LUMI
						f.Write("LUMI", 4);
						u16 = 6; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						f32 = 0;
						f32 = reverse_endian<float>(f32);
						f.Write(reinterpret_cast<char *>(&f32), 4);
						u16 = 0;
						f.Write(reinterpret_cast<char *>(&u16), 2);

						surfaceDefSize += 12;

						// DIFF
						f.Write("DIFF", 4);
						u16 = 6; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						f32 = 1;
						f32 = reverse_endian<float>(f32);
						f.Write(reinterpret_cast<char *>(&f32), 4);
						u16 = 0;
						f.Write(reinterpret_cast<char *>(&u16), 2);

						surfaceDefSize += 12;

						// SPEC
						f.Write("SPEC", 4);
						u16 = 6; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						f32 = 0;
						f32 = reverse_endian<float>(f32);
						f.Write(reinterpret_cast<char *>(&f32), 4);
						u16 = 0;
						f.Write(reinterpret_cast<char *>(&u16), 2);

						surfaceDefSize += 12;

						// REFL
						f.Write("REFL", 4);
						u16 = 6; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						ub = '\0';
						f.Write(reinterpret_cast<char *>(&ub), 1);
						f.Write(reinterpret_cast<char *>(&ub), 1);
						f.Write(reinterpret_cast<char *>(&ub), 1);
						f.Write(reinterpret_cast<char *>(&ub), 1);
						f.Write(reinterpret_cast<char *>(&ub), 1);
						f.Write(reinterpret_cast<char *>(&ub), 1);

						surfaceDefSize += 12;

						// TRAN
						f.Write("TRAN", 4);
						u16 = 6; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						ub = '\0';
						f.Write(reinterpret_cast<char *>(&ub), 1);
						f.Write(reinterpret_cast<char *>(&ub), 1);
						f.Write(reinterpret_cast<char *>(&ub), 1);
						f.Write(reinterpret_cast<char *>(&ub), 1);
						f.Write(reinterpret_cast<char *>(&ub), 1);
						f.Write(reinterpret_cast<char *>(&ub), 1);

						surfaceDefSize += 12;

						// GLOSSINESS
						f.Write("GLOS", 4);
						u16 = 6; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						
						// Value
						// try 50% gloss for 'relfection surfaces
						if (p.useEnvMap)
							f32 = 128.0;
						else
							f32 = 0.0;
						f32 = reverse_endian<float>(f32);
						f.Write(reinterpret_cast<char *>(&f32), 4);
						u16 = 0;
						f.Write(reinterpret_cast<char *>(&u16), 2);
						
						surfaceDefSize += 12;

						// RFOP
						f.Write("RFOP", 4);
						u16 = 2; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						u16 = 1;
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);

						surfaceDefSize += 8;

						// TROP
						f.Write("TROP", 4);
						u16 = 2; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						u16 = 1;
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);

						surfaceDefSize += 8;

						// SIDE
						f.Write("SIDE", 4);
						u16 = 2; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						u16 = 1;
						// If double-sided...
						if (p.cull == false){
							u16 = 3;
						}
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);

						surfaceDefSize += 8;

						// SMAN (Smoothing)
						f.Write("SMAN", 4);
						u16 = 4; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						// Smoothing is done in radiens. PI = 180 degree smoothing.
						f32 = PI;
						f32 = reverse_endian<float>(f32);
						f.Write(reinterpret_cast<char *>(&f32), 4);

						surfaceDefSize += 10;

						
						// --
						// BLOK
						uint16 blokSize = 0;
						f.Write("BLOK", 4);
						f.Write(reinterpret_cast<char *>(&blokSize), 2);
						surfaceDefSize += 6;

						// IMAP
						f.Write("IMAP", 4);
						u16 = 50-8; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						u16 = 0x80;
						f.Write(reinterpret_cast<char *>(&u16), 2);
						blokSize += 8;

						// CHAN
						f.Write("CHAN", 4);
						u16 = 4; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						f.Write("COLR", 4);
						blokSize += 10;

						// OPAC
						f.Write("OPAC", 4);
						u16 = 8; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						u16 = 0;
						f.Write(reinterpret_cast<char *>(&u16), 2);
						f32 = 1.0;
						f32 = reverse_endian<float>(f32);
						f.Write(reinterpret_cast<char *>(&f32), 4);
						u16 = 0;
						f.Write(reinterpret_cast<char *>(&u16), 2);
						blokSize += 14;

						// ENAB
						f.Write("ENAB", 4);
						u16 = 2; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						u16 = 1;
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						blokSize += 8;

						// NEGA
						f.Write("NEGA", 4);
						u16 = 2; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						u16 = 0;
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						blokSize += 8;
			/*
						// AXIS
						f.Write("AXIS", 4);
						u16 = 2; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						u16 = 1;
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						blokSize += 8;
			*/
						// TMAP
						f.Write("TMAP", 4);
						u16 = 98; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						blokSize += 6;

						// CNTR
						f.Write("CNTR", 4);
						u16 = 14; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						f32 = 0.0;
						f.Write(reinterpret_cast<char *>(&f32), 4);
						f.Write(reinterpret_cast<char *>(&f32), 4);
						f.Write(reinterpret_cast<char *>(&f32), 4);
						u16 = 0;
						f.Write(reinterpret_cast<char *>(&u16), 2);
						blokSize += 20;

						// SIZE
						f.Write("SIZE", 4);
						u16 = 14; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						f32 = 1.0;
						f32 = reverse_endian<float>(f32);
						f.Write(reinterpret_cast<char *>(&f32), 4);
						f.Write(reinterpret_cast<char *>(&f32), 4);
						f.Write(reinterpret_cast<char *>(&f32), 4);
						u16 = 0;
						f.Write(reinterpret_cast<char *>(&u16), 2);
						blokSize += 20;

						// ROTA
						f.Write("ROTA", 4);
						u16 = 14; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						f32 = 0.0;
						f.Write(reinterpret_cast<char *>(&f32), 4);
						f.Write(reinterpret_cast<char *>(&f32), 4);
						f.Write(reinterpret_cast<char *>(&f32), 4);
						u16 = 0;
						f.Write(reinterpret_cast<char *>(&u16), 2);
						blokSize += 20;

						// FALL
						f.Write("FALL", 4);
						u16 = 16; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						u16 = 0;
						f.Write(reinterpret_cast<char *>(&u16), 2);
						f32 = 0.0;
						f.Write(reinterpret_cast<char *>(&f32), 4);
						f.Write(reinterpret_cast<char *>(&f32), 4);
						f.Write(reinterpret_cast<char *>(&f32), 4);
						u16 = 0;
						f.Write(reinterpret_cast<char *>(&u16), 2);
						blokSize += 22;

						// OREF
						f.Write("OREF", 4);
						u16 = 2; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						u16 = 0;
						f.Write(reinterpret_cast<char *>(&u16), 2);
						blokSize += 8;

						// CSYS
						f.Write("CSYS", 4);
						u16 = 2; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						u16 = 0;
						f.Write(reinterpret_cast<char *>(&u16), 2);
						blokSize += 8;

						// end TMAP

						// PROJ
						f.Write("PROJ", 4);
						u16 = 2; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						u16 = 5;
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						blokSize += 8;

						// AXIS
						f.Write("AXIS", 4);
						u16 = 2; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						u16 = 2;
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						blokSize += 8;

						// IMAG
						f.Write("IMAG", 4);
						u16 = 2; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						u16 = surfaceCounter;
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						blokSize += 8;

						// WRAP
						f.Write("WRAP", 4);
						u16 = 4; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						u16 = 1;
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						u16 = 1;
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						blokSize += 10;

						// WRPW
						f.Write("WRPW", 4);
						u16 = 6; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						f32 = 1;
						f32 = reverse_endian<float>(f32);
						f.Write(reinterpret_cast<char *>(&f32), 4);
						u16 = 0;
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						blokSize += 12;

						// WRPH
						f.Write("WRPH", 4);
						u16 = 6; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						f32 = 1;
						f32 = reverse_endian<float>(f32);
						f.Write(reinterpret_cast<char *>(&f32), 4);
						u16 = 0;
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						blokSize += 12;

						// VMAP
						f.Write("VMAP", 4);
						u16 = 8; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						wxString t = _T("Texture");
						t.Append(_T('\0'));
						f.Write(t.data(), t.length());
						blokSize += 14;

						// AAST
						f.Write("AAST", 4);
						u16 = 6; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						u16 = 1;
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						f32 = 1;
						f32 = reverse_endian<float>(f32);
						f.Write(reinterpret_cast<char *>(&f32), 4);
						blokSize += 12;

						// PIXB
						f.Write("PIXB", 4);
						u16 = 2; // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						u16 = 1;
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						blokSize += 8;

						// CMNT
						f.Write("CMNT", 4);
						wxString cmnt = texarrayAtt[p.tex];
						cmnt.Append(_T('\0'));
						if (fmod((float)cmnt.length(), 2.0f) > 0)
							cmnt.Append(_T('\0'));
						u16 = cmnt.length(); // size
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						f.Write(cmnt.data(), cmnt.length());
						surfaceDefSize += 6 + cmnt.length();

						f.Write("VERS", 4);
						u16 = 4;
						u16 = MSB2(u16);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						f32 = 950;
						f32 = reverse_endian<int32>(f32);
						f.Write(reinterpret_cast<char *>(&f32), 4);
						surfaceDefSize += 10;

						// 
						surfaceDefSize += blokSize;
						off_t = -2-blokSize;
						f.SeekO(off_t, wxFromCurrent);
						u16 = MSB2(blokSize);
						f.Write(reinterpret_cast<char *>(&u16), 2);
						f.SeekO(0, wxFromEnd);
						// ================
						

						fileLen += surfaceDefSize;
						off_t = -4-surfaceDefSize;
						f.SeekO(off_t, wxFromCurrent);
						u32 = reverse_endian<uint32>(surfaceDefSize);
						f.Write(reinterpret_cast<char *>(&u32), 4);
						f.SeekO(0, wxFromEnd);
					}
				}
			}
		}
		for (size_t i=0; i<att->children.size(); i++) {
			Attachment *att2 = att->children[i];
			for (size_t j=0; j<att2->children.size(); j++) {
				Model *mAttChild = static_cast<Model*>(att2->children[j]->model);

				if (mAttChild){
					for (size_t i=0; i<mAttChild->passes.size(); i++) {
						ModelRenderPass &p = mAttChild->passes[i];

						if (p.init(mAttChild)) {
							uint32 surfaceDefSize = 0;
							f.Write("SURF", 4);
							u32 = reverse_endian<uint32>(surfaceDefSize);
							f.Write(reinterpret_cast<char *>(&u32), 4);
							fileLen += 8;

							// Surface name
							int thisslot = att2->children[j]->slot;
							if (thisslot < 15 && slots[thisslot]!=_T("")){
								surfName = wxString::Format("%s",slots[thisslot]);
							}else{
								surfName = texarrayAttc[p.tex].AfterLast('\\');
							}
							if (surfName.Len() == 0)
								surfName = wxString::Format(_T("Child %02i - Material %03i"), j, i);

							surfaceCounter++;
							
							surfName.Append(_T('\0'));
							if (fmod((float)surfName.length(), 2.0f) > 0)
								surfName.Append(_T('\0'));

							surfName.Append(_T('\0')); // ""
							surfName.Append(_T('\0'));
							f.Write(surfName.data(), (int)surfName.length());
							
							surfaceDefSize += (uint32)surfName.length();

							// Surface Attributes
							// COLOR, size 4, bytes 2
							f.Write("COLR", 4);
							u16 = 14; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							
							// value
							f32 = reverse_endian<float>(p.ocol.x);
							f.Write(reinterpret_cast<char *>(&f32), 4);
							f32 = reverse_endian<float>(p.ocol.y);
							f.Write(reinterpret_cast<char *>(&f32), 4);
							f32 = reverse_endian<float>(p.ocol.z);
							f.Write(reinterpret_cast<char *>(&f32), 4);
							u16 = 0;
							f.Write(reinterpret_cast<char *>(&u16), 2);
							
							surfaceDefSize += 20;

							// LUMI
							f.Write("LUMI", 4);
							u16 = 6; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							f32 = 0;
							f32 = reverse_endian<float>(f32);
							f.Write(reinterpret_cast<char *>(&f32), 4);
							u16 = 0;
							f.Write(reinterpret_cast<char *>(&u16), 2);

							surfaceDefSize += 12;

							// DIFF
							f.Write("DIFF", 4);
							u16 = 6; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							f32 = 1;
							f32 = reverse_endian<float>(f32);
							f.Write(reinterpret_cast<char *>(&f32), 4);
							u16 = 0;
							f.Write(reinterpret_cast<char *>(&u16), 2);

							surfaceDefSize += 12;

							// SPEC
							f.Write("SPEC", 4);
							u16 = 6; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							f32 = 0;
							f32 = reverse_endian<float>(f32);
							f.Write(reinterpret_cast<char *>(&f32), 4);
							u16 = 0;
							f.Write(reinterpret_cast<char *>(&u16), 2);

							surfaceDefSize += 12;

							// REFL
							f.Write("REFL", 4);
							u16 = 6; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							ub = '\0';
							f.Write(reinterpret_cast<char *>(&ub), 1);
							f.Write(reinterpret_cast<char *>(&ub), 1);
							f.Write(reinterpret_cast<char *>(&ub), 1);
							f.Write(reinterpret_cast<char *>(&ub), 1);
							f.Write(reinterpret_cast<char *>(&ub), 1);
							f.Write(reinterpret_cast<char *>(&ub), 1);

							surfaceDefSize += 12;

							// TRAN
							f.Write("TRAN", 4);
							u16 = 6; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							ub = '\0';
							f.Write(reinterpret_cast<char *>(&ub), 1);
							f.Write(reinterpret_cast<char *>(&ub), 1);
							f.Write(reinterpret_cast<char *>(&ub), 1);
							f.Write(reinterpret_cast<char *>(&ub), 1);
							f.Write(reinterpret_cast<char *>(&ub), 1);
							f.Write(reinterpret_cast<char *>(&ub), 1);

							surfaceDefSize += 12;

							// GLOSSINESS
							f.Write("GLOS", 4);
							u16 = 6; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							
							// Value
							// try 50% gloss for 'relfection surfaces
							if (p.useEnvMap)
								f32 = 128.0;
							else
								f32 = 0.0;
							f32 = reverse_endian<float>(f32);
							f.Write(reinterpret_cast<char *>(&f32), 4);
							u16 = 0;
							f.Write(reinterpret_cast<char *>(&u16), 2);
							
							surfaceDefSize += 12;

							// RFOP
							f.Write("RFOP", 4);
							u16 = 2; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							u16 = 1;
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);

							surfaceDefSize += 8;

							// TROP
							f.Write("TROP", 4);
							u16 = 2; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							u16 = 1;
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);

							surfaceDefSize += 8;

							// SIDE
							f.Write("SIDE", 4);
							u16 = 2; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							u16 = 1;
							// If double-sided...
							if (p.cull == false){
								u16 = 3;
							}
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);

							surfaceDefSize += 8;

							// SMAN (Smoothing)
							f.Write("SMAN", 4);
							u16 = 4; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							// Smoothing is done in radiens. PI = 180 degree smoothing.
							f32 = PI;
							f32 = reverse_endian<float>(f32);
							f.Write(reinterpret_cast<char *>(&f32), 4);

							surfaceDefSize += 10;

							
							// --
							// BLOK
							uint16 blokSize = 0;
							f.Write("BLOK", 4);
							f.Write(reinterpret_cast<char *>(&blokSize), 2);
							surfaceDefSize += 6;

							// IMAP
							f.Write("IMAP", 4);
							u16 = 50-8; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							u16 = 0x80;
							f.Write(reinterpret_cast<char *>(&u16), 2);
							blokSize += 8;

							// CHAN
							f.Write("CHAN", 4);
							u16 = 4; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							f.Write("COLR", 4);
							blokSize += 10;

							// OPAC
							f.Write("OPAC", 4);
							u16 = 8; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							u16 = 0;
							f.Write(reinterpret_cast<char *>(&u16), 2);
							f32 = 1.0;
							f32 = reverse_endian<float>(f32);
							f.Write(reinterpret_cast<char *>(&f32), 4);
							u16 = 0;
							f.Write(reinterpret_cast<char *>(&u16), 2);
							blokSize += 14;

							// ENAB
							f.Write("ENAB", 4);
							u16 = 2; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							u16 = 1;
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							blokSize += 8;

							// NEGA
							f.Write("NEGA", 4);
							u16 = 2; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							u16 = 0;
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							blokSize += 8;
				/*
							// AXIS
							f.Write("AXIS", 4);
							u16 = 2; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							u16 = 1;
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							blokSize += 8;
				*/
							// TMAP
							f.Write("TMAP", 4);
							u16 = 98; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							blokSize += 6;

							// CNTR
							f.Write("CNTR", 4);
							u16 = 14; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							f32 = 0.0;
							f.Write(reinterpret_cast<char *>(&f32), 4);
							f.Write(reinterpret_cast<char *>(&f32), 4);
							f.Write(reinterpret_cast<char *>(&f32), 4);
							u16 = 0;
							f.Write(reinterpret_cast<char *>(&u16), 2);
							blokSize += 20;

							// SIZE
							f.Write("SIZE", 4);
							u16 = 14; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							f32 = 1.0;
							f32 = reverse_endian<float>(f32);
							f.Write(reinterpret_cast<char *>(&f32), 4);
							f.Write(reinterpret_cast<char *>(&f32), 4);
							f.Write(reinterpret_cast<char *>(&f32), 4);
							u16 = 0;
							f.Write(reinterpret_cast<char *>(&u16), 2);
							blokSize += 20;

							// ROTA
							f.Write("ROTA", 4);
							u16 = 14; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							f32 = 0.0;
							f.Write(reinterpret_cast<char *>(&f32), 4);
							f.Write(reinterpret_cast<char *>(&f32), 4);
							f.Write(reinterpret_cast<char *>(&f32), 4);
							u16 = 0;
							f.Write(reinterpret_cast<char *>(&u16), 2);
							blokSize += 20;

							// FALL
							f.Write("FALL", 4);
							u16 = 16; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							u16 = 0;
							f.Write(reinterpret_cast<char *>(&u16), 2);
							f32 = 0.0;
							f.Write(reinterpret_cast<char *>(&f32), 4);
							f.Write(reinterpret_cast<char *>(&f32), 4);
							f.Write(reinterpret_cast<char *>(&f32), 4);
							u16 = 0;
							f.Write(reinterpret_cast<char *>(&u16), 2);
							blokSize += 22;

							// OREF
							f.Write("OREF", 4);
							u16 = 2; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							u16 = 0;
							f.Write(reinterpret_cast<char *>(&u16), 2);
							blokSize += 8;

							// CSYS
							f.Write("CSYS", 4);
							u16 = 2; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							u16 = 0;
							f.Write(reinterpret_cast<char *>(&u16), 2);
							blokSize += 8;

							// end TMAP

							// PROJ
							f.Write("PROJ", 4);
							u16 = 2; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							u16 = 5;
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							blokSize += 8;

							// AXIS
							f.Write("AXIS", 4);
							u16 = 2; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							u16 = 2;
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							blokSize += 8;

							// IMAG
							f.Write("IMAG", 4);
							u16 = 2; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							u16 = surfaceCounter;
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							blokSize += 8;

							// WRAP
							f.Write("WRAP", 4);
							u16 = 4; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							u16 = 1;
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							u16 = 1;
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							blokSize += 10;

							// WRPW
							f.Write("WRPW", 4);
							u16 = 6; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							f32 = 1;
							f32 = reverse_endian<float>(f32);
							f.Write(reinterpret_cast<char *>(&f32), 4);
							u16 = 0;
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							blokSize += 12;

							// WRPH
							f.Write("WRPH", 4);
							u16 = 6; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							f32 = 1;
							f32 = reverse_endian<float>(f32);
							f.Write(reinterpret_cast<char *>(&f32), 4);
							u16 = 0;
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							blokSize += 12;

							// VMAP
							f.Write("VMAP", 4);
							u16 = 8; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							wxString t = _T("Texture");
							t.Append(_T('\0'));
							f.Write(t.data(), t.length());
							blokSize += 14;

							// AAST
							f.Write("AAST", 4);
							u16 = 6; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							u16 = 1;
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							f32 = 1;
							f32 = reverse_endian<float>(f32);
							f.Write(reinterpret_cast<char *>(&f32), 4);
							blokSize += 12;

							// PIXB
							f.Write("PIXB", 4);
							u16 = 2; // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							u16 = 1;
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							blokSize += 8;

							// CMNT
							f.Write("CMNT", 4);
							wxString cmnt = wxString::Format("%s",slots[thisslot]);
							cmnt.Append(_T('\0'));
							if (fmod((float)cmnt.length(), 2.0f) > 0)
								cmnt.Append(_T('\0'));
							u16 = cmnt.length(); // size
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							f.Write(cmnt.data(), cmnt.length());
							surfaceDefSize += 6 + cmnt.length();

							f.Write("VERS", 4);
							u16 = 4;
							u16 = MSB2(u16);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							f32 = 950;
							f32 = reverse_endian<int32>(f32);
							f.Write(reinterpret_cast<char *>(&f32), 4);
							surfaceDefSize += 10;

							// 
							surfaceDefSize += blokSize;
							off_t = -2-blokSize;
							f.SeekO(off_t, wxFromCurrent);
							u16 = MSB2(blokSize);
							f.Write(reinterpret_cast<char *>(&u16), 2);
							f.SeekO(0, wxFromEnd);
							// ================
							

							fileLen += surfaceDefSize;
							off_t = -4-surfaceDefSize;
							f.SeekO(off_t, wxFromCurrent);
							u32 = reverse_endian<uint32>(surfaceDefSize);
							f.Write(reinterpret_cast<char *>(&u32), 4);
							f.SeekO(0, wxFromEnd);
						}
					}
				}
			}
		}
	}
	// ================
#ifdef _DEBUG
	wxLogMessage("M2 Surface Data Written for %s",wxString(m->fullname));
#endif

	f.SeekO(4, wxFromStart);
	u32 = reverse_endian<uint32>(fileLen);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	f.SeekO(0, wxFromEnd);

	f.Close();
#ifdef _DEBUG
	wxLogMessage("M2 %s Successfully written!",wxString(m->fullname));
#endif
}


//---------------------------------------------
// --==WMOs==--
//---------------------------------------------

/*
	This will export Lights & Doodads (as nulls) into a Lightwave Scene file.
*/
void ExportWMOObjectstoLWO(WMO *m, const char *fn){
	// Should we generate a scene file?
	// Wll only generate if there are doodads or lights.
	bool doreturn = false;
	if (((modelExport_LW_ExportLights == false) || (m->nLights == 0)) && ((modelExport_LW_ExportDoodads == false) || (m->nDoodads == 0))){
		doreturn = true;
	}
	if (doreturn == true)
		return;

	// Open file
	wxString SceneName = wxString(fn, wxConvUTF8).BeforeLast('.');
	SceneName << ".lws";

	if (modelExport_LW_PreserveDir == true){
		wxString Path, Name;

		Path << SceneName.BeforeLast('\\');
		Name << SceneName.AfterLast('\\');

		MakeDirs(Path,"Scenes");

		SceneName.Empty();
		SceneName << Path << "\\Scenes\\" << Name;
	}
	if (modelExport_PreserveDir == true){
		wxString Path1, Path2, Name;
		Path1 << SceneName.BeforeLast('\\');
		Name << SceneName.AfterLast('\\');
		Path2 << wxString(m->name.c_str()).BeforeLast('\\');

		MakeDirs(Path1,Path2);

		SceneName.Empty();
		SceneName << Path1 << '\\' << Path2 << '\\' << Name;
	}

	ofstream fs(SceneName.mb_str(), ios_base::out | ios_base::trunc);

	if (!fs.is_open()) {
		wxLogMessage(_T("Error: Unable to open file '%s'. Could not export the scene."), fn);
		return;
	}
	SceneName = SceneName.AfterLast('\\');

	/*
		Lightwave Scene files are simple text files. New Lines (\n) are need to add a new variable for the scene to understand.
		Lightwave files are pretty sturdy. Variables not already in a scene or model file, will usually be generated when opened.
		As such, we don't need to declare EVERY variable for the scene file, but I'm gonna add what I think is pertinent.
	*/

	// File Top
	fs << "LWSC\n";
	fs << "5\n\n"; // I think this is a version-compatibility number...

	// Scene File Basics
	fs << "RenderRangeType 0\n";
	fs << "FirstFrame 1\n";
	fs << "LastFrame 60\n";
	fs << "FrameStep 1\n";
	fs << "RenderRangeObject 0\n";
	fs << "RenderRangeArbitrary 1-60\n";
	fs << "PreviewFirstFrame 0\n";
	fs << "PreviewLastFrame 60\n";
	fs << "PreviewFrameStep 1\n";
	fs << "CurrentFrame 0\n";
	fs << "FramesPerSecond 30\n";
	fs << "ChangeScene 0\n\n";

	int mcount = 0;	// Model Count
	int lcount = 0; // Light Count

	int DoodadLightArrayID[3000];
	int DoodadLightArrayDDID[3000];
	int DDLArrCount = 0;

	Vec3D ZeroPos(0,0,0);
	Vec3D ZeroRot(0,0,0);

	// Objects/Doodads go here

	// Exported Object
	int ModelID = mcount;
	wxString Obj = wxString(fn, wxConvUTF8).AfterLast('\\');
	wxString objFilename = "";
	if (modelExport_LW_PreserveDir == true){
		objFilename += "Objects/";
	}
	if (modelExport_PreserveDir == true){
		objFilename += wxString(m->name.c_str()).BeforeLast('\\');
		objFilename << "\\";
		objFilename.Replace("\\","/");
	}
	objFilename += Obj;

	WriteLWSceneObject(fs,objFilename,ZeroPos,ZeroRot,1,mcount);

	if (modelExport_LW_ExportDoodads ==  true){
		// Doodads
		for (int ds=0;ds<m->nDoodadSets;ds++){			
			m->showDoodadSet(ds);
			WMODoodadSet *DDSet = &m->doodadsets[ds];
			wxString DDSetName;
			DDSetName << wxString(m->name.c_str()).AfterLast('\\').BeforeLast('.') << " DoodadSet " << wxString(DDSet->name);
			int DDSID = mcount;

			WriteLWSceneObject(fs,DDSetName,ZeroPos,ZeroRot,1,mcount,true,ModelID);

			for (int dd=DDSet->start;dd<(DDSet->start+DDSet->size);dd++){
				WMOModelInstance *doodad = &m->modelis[dd];
				wxString name = wxString(doodad->filename.c_str()).AfterLast('\\').BeforeLast('.');
				// Position
				Vec3D Pos = doodad->pos;
				// Heading, Pitch & Bank.
				Vec3D Rot = QuaternionToXYZ(doodad->dir,doodad->w);

				int DDID = mcount;
				bool isNull = true;
				if (modelExport_LW_DoodadsAs > 0)
					isNull = false;
				
				if (!doodad->model){
					isNull = true;
				}

				if (isNull == false){
					wxString pathdir = "";
					if (modelExport_LW_PreserveDir == true){
						pathdir += "Objects/";
					}
					name = pathdir << wxString(doodad->filename.c_str()).BeforeLast('.') << ".lwo";
					name.Replace("\\","/");
				}

				WriteLWSceneObject(fs,name,Pos,Rot,doodad->sc,mcount,isNull,DDSID,doodad->filename.c_str());
				wxLogMessage("Export: Finished writing the Doodad to the Scene File.");

				// Doodad Lights
				// Apparently, Doodad Lights must be parented to the Doodad for proper placement.
				if ((doodad->model) && (doodad->model->header.nLights > 0)){
					wxLogMessage("Export: Doodad Lights found for %s, Number of lights: %i",wxString(doodad->filename.c_str()),doodad->model->header.nLights);
					DoodadLightArrayDDID[DDLArrCount] = DDID;
					DoodadLightArrayID[DDLArrCount] = dd;
					DDLArrCount++;
				}
			}
		}
	}

	// Lighting Basics
	fs << "AmbientColor 1 1 1\nAmbientIntensity 0.25\nDoubleSidedAreaLights 1\n\n";

	// Lights
	if (modelExport_LW_ExportLights == true){
		// WMO Lights
		for (int i=0;i<m->nLights;i++){
			WMOLight *light = &m->lights[i];

			Vec3D color;
			color.x=light->fcolor.x;
			color.y=light->fcolor.y;
			color.z=light->fcolor.z;
			float intense = light->intensity;
			bool useAtten = false;
			float AttenEnd = light->attenEnd;

			if (light->useatten > 0){
				useAtten = true;
			}

			WriteLWSceneLight(fs,lcount,light->pos,light->type,color,intense,useAtten,AttenEnd,2.5);
		}

		// Doodad Lights
		for (int i=0;i<DDLArrCount;i++){
			
			WMOModelInstance *doodad = &m->modelis[DoodadLightArrayID[i]];
			ModelLight *light = &doodad->model->lights[i];

			if (light->type < 0){
				continue;
			}
			
			Vec3D color = light->diffColor.getValue(0,0);
			float intense = light->diffIntensity.getValue(0,0);
			bool useAtten = false;
			float AttenEnd = light->AttenEnd.getValue(0,0);
			wxString name = wxString(doodad->filename.c_str()).AfterLast('\\');

			if (light->UseAttenuation.getValue(0,0) > 0){
				useAtten = true;
			}
			
			WriteLWSceneLight(fs,lcount,light->pos,light->type,color,intense,useAtten,AttenEnd,5,name,DoodadLightArrayDDID[i]);
		}
	}

	// Camera data (if we want it) goes here.
	// Yes, we can export flying cameras to Lightwave!
	// Just gotta add them back into the listing...
	fs << "AddCamera 30000000\nCameraName Camera\nShowCamera 1 -1 0.125490 0.878431 0.125490\nZoomFactor 2.316845\nZoomType 2\n\n";
	WriteLWScenePlugin(fs,"CameraHandler",1,"Perspective");	// Make the camera a Perspective camera

	// Backdrop Settings
	// Add this if viewing a skybox, or using one as a background?

	// Rendering Options
	// Raytrace Shadows enabled.
	fs << "RenderMode 2\nRayTraceEffects 1\nDepthBufferAA 0\nRenderLines 1\nRayRecursionLimit 16\nRayPrecision 6\nRayCutoff 0.01\nDataOverlayLabel  \nSaveRGB 0\nSaveAlpha 0\n";

	fs.close();

	// Export Doodad Files
	wxString cWMOName(m->name.c_str());
	if (modelExport_LW_ExportDoodads ==  true){
		if (modelExport_LW_DoodadsAs == 1){
			// Copy Model-list into an array
			std::vector<std::string> modelarray = m->models;

			// Remove the WMO
			wxDELETE(g_modelViewer->canvas->wmo);
			g_modelViewer->canvas->wmo = NULL;
			g_modelViewer->isWMO = false;
			g_modelViewer->isChar = false;
			
			// Export Individual Doodad Models
			for (int x=0;x<modelarray.size();x++){
				g_modelViewer->isModel = true;
				wxString cModelName(modelarray[x].c_str());

				wxLogMessage("Export: Attempting to export doodad model: %s",cModelName);
				wxString dfile = wxString(fn).BeforeLast('\\') << '\\' << cModelName.AfterLast('\\');
				dfile = dfile.BeforeLast('.') << ".lwo";

				g_modelViewer->canvas->LoadModel(cModelName.c_str());
				ExportM2toLWO(NULL, g_modelViewer->canvas->model, dfile.fn_str(), true);

				wxLogMessage("Export: Finished exporting doodad model: %s",cModelName);

				// Delete the loaded model
				g_modelViewer->canvas->clearAttachments();
				g_modelViewer->canvas->model = NULL;
				g_modelViewer->isModel = false;
			}
		}
		texturemanager.clear();

		// Reload our original WMO file.
		//wxLogMessage("Reloading original WMO file: %s",cWMOName);

		// Load the WMO


	}
}


void ExportWMOtoLWO(WMO *m, const char *fn){
	wxString file = wxString(fn, wxConvUTF8);

	if (modelExport_LW_PreserveDir == true){
		wxString Path, Name;

		Path << file.BeforeLast('\\');
		Name << file.AfterLast('\\');

		MakeDirs(Path,"Objects");

		file.Empty();
		file << Path << "\\Objects\\" << Name;
	}
	if (modelExport_PreserveDir == true){
		wxString Path1, Path2, Name;
		Path1 << file.BeforeLast('\\');
		Name << file.AfterLast('\\');
		Path2 << wxString(m->name.c_str()).BeforeLast('\\');

		MakeDirs(Path1,Path2);

		file.Empty();
		file << Path1 << '\\' << Path2 << '\\' << Name;
	}

	wxFFileOutputStream f(file, wxT("w+b"));

	if (!f.IsOk()) {
		wxLogMessage(_T("Error: Unable to open file '%s'. Could not export model."), file);
		wxMessageDialog(g_modelViewer,"Could not open file for exporting.","Exporting Error...");
		return;
	}

	int off_t;
	uint16 dimension;

	// LightWave object files use the IFF syntax described in the EA-IFF85 document. Data is stored in a collection of chunks. 
	// Each chunk begins with a 4-byte chunk ID and the size of the chunk in bytes, and this is followed by the chunk contents.


	/* LWO Model Format, as layed out in offical LWO2 files.( I Hex Edited to find most of this information from files I made/saved in Lightwave. -Kjasi)
	FORM	// Format Declaration
	LWO2	// Declares this is the Lightwave Object 2 file format. LWOB is the first format. Doesn't have a lot of the cool stuff LWO2 has...

	TAGS	// Used for various Strings
		Sketch Color Names
		Part Names
		Surface Names
	LAYR		// Specifies the start of a new layer. Probably best to only be on one...
		PNTS		// Points listing & Block Section
			BBOX		// Bounding Box. It's optional, but will probably help.
			VMPA		// Vertex Map Parameters, Always Preceeds a VMAP & VMAD. 4bytes: Size (2 * 4 bytes).
						// UV Sub Type: 0-Linear, 1-Subpatched, 2-Linear Corners, 3-Linear Edges, 4-Across Discontinuous Edges.
						// Sketch Color: 0-12; 6-Default Gray
				VMAP		// Vector Map Section. Always Preceeds the following:
					SPOT	// Aboslute Morph Maps. Used only while modeling. Ignore.
					TXUV	// Defines UV Vector Map. Best not to use these unless the data has no Discontinuous UVs.
					PICK	// Point Selection Sets (2 bytes, then Set Name, then data. (Don't know what kind)
					MORF	// Relative Morph Maps. These are used for non-boned mesh animation.
					RGB		// Point Color Map, no Alpha. Note the space at end of the group!
					RGBA	// Same as above, but with an alpha channel.
					WGHT	// Weight Map. Used to give bones limited areas to effect, or can be used for point-by-point maps for various surfacing tricks.

		POLS		// Declares Polygon section. Next 4 bytes = Number of Polys
			FACE		// The actual Polygons. The maximum number of vertices is 1023 per poly!
				PTAG		// The Poly Tags for this Poly. These usually reference items in the TAGS group.
					COLR	// The Sketch Color Name
					PART	// The Part Name
					SURF	// The Surface Name
				VMPA		// Discontinuous Vertex Map Parameters (See the one in the Points section for details)
					VMAD		// Discontinuous Vector Map Section. Best if used only for UV Maps. Difference between VMAP & VMAD: VMAPs are connected to points, while VMADs are connected to Polys.
						APSL	// Adaptive Pixel Subdivision Level. Only needed for sub-patched models, so just ignore it for our outputs.
						TXUV	// Defines UV Vector Map
			PTCH	// Cat-mull Clarke Patches. Don't need this, but it mirror's FACE's sub-chunks.
			SUBD	// Subdivision Patches. Same as above.
			MBAL	// Metaballs. Don't bother...
			BONE	// Line segments representing the object's skeleton. These are converted to bones for deformation during rendering.

	CLIP (for each Image)
		STIL	// 2 bytes, size of string, followed by image name.extention
		FLAG	// Flags. 2 bytes, size of chunk. Not sure what the flag values are.
		AMOD	// 2 bytes: What's changed, 2 bytes: value. 2-Alphas: 0-Enabled, 1-Disabled, 2-Alpha Only. AMOD is omitted if value is 0.
	XREF	// Calls an instance of a CLIP. We'll avoid this for now.

	SURF	// Starts the surface's data. Not sure about the 4 bytes after it...
		// Until BLOK, this just sets the default values
		COLR	// Color
		LUMI	// Luminosity
		DIFF	// Diffusion
		SPEC	// Specularity
		REFL	// Reflections
		TRAN	// Transparancy
		TRNL	// Translucency
		RIND	// Refractive Index
		BUMP	// Bump Amount
		GLOS	// Glossiness
		GVAL	// Glow
		SHRP	// Diffuse Sharpness

		SMAN	// Smoothing Amount

		RFOP	// Reflection Options: 0-Backdrop Only (default), 1-Raytracing + Backdrop, 2 - Spherical Map, 3 - Raytracing + Spherical Map
		TROP	// Same as RFOP, but for Refraction.
		SIDE	// Is it Double-Sided?
		NVSK	// Exclude from VStack

		CMNT // Surface Comment, but I don't seem to be able to get it to show up in LW... 2bytes: Size. Simple Text line for this surface. Make sure it doesn't end on an odd byte! VERS must be 931 or 950!
		VERS // Version Compatibility mode, including what it's compatible with. 2 bytes (int16, value 4), 4 bytes (int32, value is 850 for LW8.5 Compatability, 931 for LW9.3.1, and 950 for Default)

		BLOK	// First Blok. Bloks hold Surface texture information!
			IMAP	// Declares that this surface texture is an image map.
				CHAN COLR	// Declares that the image map will be applied to the color channel. (Color has a Texture!)
					OPAC	// Opacity of Layer
					ENAB	// Is the layer enabled?
					NEGA	// Is it inverted?
					TMAP	// Texture Map details
						CNTR	// Position
						SIZE	// Scale
						ROTA	// Rotation
						FALL	// Falloff
						OREF	// Object Reference
						CSYS	// Coordinate System: 0-Object's Coordinates, 1-World's Coordinates

						// Image Maps
						PROJ	// Image Projection Mode: 0-Planar (references AXIS), 1-Cylindrical, 2-Spherical, 3-Cubic, 4-Front Projection, 5-UV (IDed in VMAP chunk)
						AXIS	// The axis the image uses: 0-X, 1-Y, or 2-Z;
						IMAG	// The image to use: Use CLIP Index
						WRAP	// Wrapping Mode: 0-Reset, 1-Repeat, 2-Mirror, 3-Edge
						WRPW	// Wrap Count Width (Used for Cylindrical & Spherical projections)
						WRPH	// Wrap Count Height
						VMAP	// Name of the UV Map to use, should PROJ be set to 5!
						AAST	// Antialiasing Strength
						PIXB	// Pixel Blending

		// Node Information
		// We can probably skip this for now. Later, it would be cool to mess with it, but for now, it'll be automatically generated once the file is opened in LW.

		NODS	// Node Block & Size
			NROT
			NLOC
			NZOM
			NSTA	// Activate Nodes
			NVER
			NNDS
			NSRV
				Surface
			NTAG
			NRNM
				Surface
			NNME
				Surface
			NCRD
			NMOD
			NDTA
			NPRW
			NCOM
			NCON

	*/




	unsigned int fileLen = 0;


	// ===================================================
	// FORM		// Format Declaration
	//
	// Always exempt from the length of the file!
	// ===================================================
	f.Write("FORM", 4);
	f.Write(reinterpret_cast<char *>(&fileLen), 4);


	// ===================================================
	// LWO2
	//
	// Declares this is the Lightwave Object 2 file format.
	// LWOB is the first format. It doesn't have a lot of the cool stuff LWO2 has...
	// ===================================================
	f.Write("LWO2", 4);
	fileLen += 4;


	// ===================================================
	// TAGS
	//
	// Used for various Strings. Known string types, in order:
	//		Sketch Color Names
	//		Part Names
	//		Surface Names
	// ===================================================
	f.Write("TAGS", 4);
	uint32 tagsSize = 0;
	u32 = 0;
	f.Write(reinterpret_cast<char *>(&u32), 4);
	fileLen += 8;

	uint32 counter=0;
	uint32 TagCounter=0;
	uint16 PartCounter=0;
	uint16 SurfCounter=0;
	unsigned int numVerts = 0;
	unsigned int numGroups = 0;

	// Build Surface Name Database
	/*
	std::vector<std::string> surfarray;
	for (uint16 t=0;t<m->nTextures;t++){
		surfarray.push_back(wxString(m->textures[t]).BeforeLast('.').c_str());
	}
	*/

	// Texture Name Array
	// Find a Match for mat->tex and place it into the Texture Name Array.
	wxString *texarray = new wxString[m->nTextures+1];
	for (int g=0;g<m->nGroups;g++) {
		for (int b=0; b<m->groups[g].nBatches; b++)
		{
			WMOBatch *batch = &m->groups[g].batches[b];
			WMOMaterial *mat = &m->mat[batch->texture];
			wxString outname(fn, wxConvUTF8);

			bool nomatch = true;
			for (int t=0;t<=m->nTextures; t++) {
				if (t == mat->tex) {
					texarray[mat->tex] = m->textures[t-1].c_str();
					texarray[mat->tex] = texarray[mat->tex].BeforeLast('.');
					nomatch = false;
					break;
				}
			}
			if (nomatch == true){
				texarray[mat->tex] = outname << wxString::Format(_T("_Material_%03i"), mat->tex);
			}
		}
	}

	// --== Part Names ==--
	for (int g=0;g<m->nGroups;g++) {
		wxString partName = m->groups[g].name.c_str();

		partName.Append(_T('\0'));
		if (fmod((float)partName.length(), 2.0f) > 0)
			partName.Append(_T('\0'));
		f.Write(partName.data(), partName.length());
		tagsSize += partName.length();
	}

	// --== Surface Names ==--
	wxString *surfarray = new wxString[m->nTextures+1];
	for (unsigned int t=0;t<m->nTextures;t++){
		wxString matName = wxString(m->textures[t].c_str()).AfterLast('\\').BeforeLast('.');

		matName.Append(_T('\0'));
		if (fmod((float)matName.length(), 2.0f) > 0)
			matName.Append(_T('\0'));
		f.Write(matName.data(), matName.length());
		tagsSize += matName.length();
		surfarray[t] = wxString(m->textures[t].c_str()).BeforeLast('.');
	}

	off_t = -4-tagsSize;
	f.SeekO(off_t, wxFromCurrent);
	u32 = reverse_endian<uint32>(tagsSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	f.SeekO(0, wxFromEnd);
	fileLen += tagsSize;



	// ===================================================
	// LAYR
	//
	// Specifies the start of a new layer. Each layer has it's own Point & Poly
	// chunk, which tells it what data is on what layer. It's probably best
	// to only have 1 layer for now.
	// ===================================================
	f.Write("LAYR", 4);
	u32 = reverse_endian<uint32>(18);
	fileLen += 8;
	f.Write(reinterpret_cast<char *>(&u32), 4);
	ub = 0;
	for(int i=0; i<18; i++) {
		f.Write(reinterpret_cast<char *>(&ub), 1);
	}
	fileLen += 18;
	// ================


	// ===================================================
	// POINTS CHUNK, this is the vertice data
	// The PNTS chunk contains triples of floating-point numbers, the coordinates of a list of points. The numbers are written 
	// as IEEE 32-bit floats in network byte order. The IEEE float format is the standard bit pattern used by almost all CPUs 
	// and corresponds to the internal representation of the C language float type. In other words, this isn't some bizarre 
	// proprietary encoding. You can process these using simple fread and fwrite calls (but don't forget to correct the byte 
	// order if necessary).
	// ===================================================
	uint32 pointsSize = 0;
	f.Write("PNTS", 4);
	u32 = reverse_endian<uint32>(pointsSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	fileLen += 8;

	unsigned int bindice = 0;
	unsigned int gverts = 0;

	// output all the vertice data
	for (int g=0; g<m->nGroups; g++) {
		gverts += m->groups[g].nVertices;
		for (int b=0; b<m->groups[g].nBatches; b++)	{
			WMOBatch *batch = &m->groups[g].batches[b];

			bindice += batch->indexCount;
			for(int ii=0;ii<batch->indexCount;ii++){
				int a = m->groups[g].indices[batch->indexStart + ii];
				Vec3D vert;
				vert.x = reverse_endian<float>(m->groups[g].vertices[a].x);
				vert.y = reverse_endian<float>(m->groups[g].vertices[a].z);
				vert.z = reverse_endian<float>(m->groups[g].vertices[a].y);
				f.Write(reinterpret_cast<char *>(&vert.x), 4);
				f.Write(reinterpret_cast<char *>(&vert.y), 4);
				f.Write(reinterpret_cast<char *>(&vert.z), 4);
				pointsSize += 12;

				numVerts++;
			}
			numGroups++;
		}
	}
#ifdef _DEBUG
	wxLogMessage("WMO Point Count: %i, Stored Indices: %i, Stored Verticies: %i",numVerts, bindice, gverts);
#endif


	fileLen += pointsSize;
	off_t = -4-pointsSize;
	f.SeekO(off_t, wxFromCurrent);
	u32 = reverse_endian<uint32>(pointsSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	f.SeekO(0, wxFromEnd);
	// ================


	// --
	// The bounding box for the layer, just so that readers don't have to scan the PNTS chunk to find the extents.
	f.Write("BBOX", 4);
	u32 = reverse_endian<uint32>(24);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	Vec3D vert;
	vert.x = reverse_endian<float>(m->v1.x);
	vert.z = reverse_endian<float>(m->v1.y);
	vert.y = reverse_endian<float>(m->v1.z);
	f.Write(reinterpret_cast<char *>(&vert.x), 4);
	f.Write(reinterpret_cast<char *>(&vert.y), 4);
	f.Write(reinterpret_cast<char *>(&vert.z), 4);
	vert.x = reverse_endian<float>(m->v2.x);
	vert.z = reverse_endian<float>(m->v2.y);
	vert.y = reverse_endian<float>(m->v2.z);
	f.Write(reinterpret_cast<char *>(&vert.x), 4);
	f.Write(reinterpret_cast<char *>(&vert.y), 4);
	f.Write(reinterpret_cast<char *>(&vert.z), 4);
	fileLen += 32;



	// Removed Point Vertex Mapping for WMOs.
	// UV Map now generated by Discontinuous Vertex Mapping, down in the Poly section.

	uint32 vmapSize = 0;

	//Vertex Mapping
	f.Write("VMAP", 4);
	u32 = reverse_endian<uint32>(vmapSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	fileLen += 8;
	// UV Data
	f.Write("TXUV", 4);
	dimension = MSB2(2);
	f.Write(reinterpret_cast<char *>(&dimension), 2);
	f.Write("Texture", 7);
	ub = 0;
	f.Write(reinterpret_cast<char *>(&ub), 1);
	vmapSize += 14;

	// u16, f32, f32
	for (int g=0; g<m->nGroups; g++) {
		for (int b=0; b<m->groups[g].nBatches; b++)
		{
			WMOBatch *batch = &m->groups[g].batches[b];
			for(int ii=0;ii<batch->indexCount;ii++)
			{
				int a = m->groups[g].indices[batch->indexStart + ii];

				uint16 indice16;
				uint32 indice32;

				uint16 counter16 = (counter & 0x0000FFFF);
				uint32 counter32 = counter + 0xFF000000;

				if (counter < 0xFF00){
					indice16 = MSB2(counter16);
					f.Write(reinterpret_cast<char *>(&indice16),2);
					vmapSize += 2;
				}else{
					indice32 = reverse_endian<uint32>(counter32);
					f.Write(reinterpret_cast<char *>(&indice32), 4);
					vmapSize += 4;
				}

				f32 = reverse_endian<float>(m->groups[g].texcoords[a].x);
				f.Write(reinterpret_cast<char *>(&f32), 4);
				f32 = reverse_endian<float>(1 - m->groups[g].texcoords[a].y);
				f.Write(reinterpret_cast<char *>(&f32), 4);
				vmapSize += 8;
				counter++;
			}
		}
	}
	fileLen += vmapSize;

	off_t = -4-vmapSize;
	f.SeekO(off_t, wxFromCurrent);
	u32 = reverse_endian<uint32>(vmapSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	f.SeekO(0, wxFromEnd);




	// --
	// POLYGON CHUNK
	// The POLS chunk contains a list of polygons. A "polygon" in this context is anything that can be described using an 
	// ordered list of vertices. A POLS of type FACE contains ordinary polygons, but the POLS type can also be CURV, 
	// PTCH, MBAL or BONE, for example.
	//
	// The high 6 bits of the vertex count for each polygon are reserved for flags, which in effect limits the number of 
	// vertices per polygon to 1023. Don't forget to mask the high bits when reading the vertex count. The flags are 
	// currently only defined for CURVs.
	// 
	// The point indexes following the vertex count refer to the points defined in the most recent PNTS chunk. Each index 
	// can be a 2-byte or a 4-byte integer. If the high order (first) byte of the index is not 0xFF, the index is 2 bytes long. 
	// This allows values up to 65279 to be stored in 2 bytes. If the high order byte is 0xFF, the index is 4 bytes long and 
	// its value is in the low three bytes (index & 0x00FFFFFF). The maximum value for 4-byte indexes is 16,777,215 (224 - 1). 
	// Objects with more than 224 vertices can be stored using multiple pairs of PNTS and POLS chunks.
	// 
	// The cube has 6 square faces each defined by 4 vertices. LightWave polygons are single-sided by default 
	// (double-sidedness is a possible surface property). The vertices are listed in clockwise order as viewed from the 
	// visible side, starting with a convex vertex. (The normal is defined as the cross product of the first and last edges.)

	f.Write("POLS", 4);
	uint32 polySize = 4;
	u32 = reverse_endian<uint32>(polySize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	fileLen += 8; // FACE is handled in the PolySize
	f.Write("FACE", 4);

	counter = 0;
	unsigned long polycount = 0;
	uint32 gpolys = 0;
	unsigned int tnBatches = 0;
	
	for (int g=0;g<m->nGroups;g++) {
		gpolys += m->groups[g].nTriangles;
		for (int b=0; b<m->groups[g].nBatches; b++){
			WMOBatch *batch = &m->groups[g].batches[b];
			for (unsigned int k=0; k<batch->indexCount; k+=3) {
				uint16 nverts;

				// Write the number of Verts
				nverts = MSB2(3);
				f.Write(reinterpret_cast<char *>(&nverts),2);
				polySize += 2;

				for (int x=0;x<3;x++,counter++){
					//wxLogMessage("Batch %i, index %i, x=%i",b,k,x);
					uint16 indice16;
					uint32 indice32;

					int mod = 0;
					if (x==1){
						mod = 1;
					}else if (x==2){
						mod = -1;
					}

					uint16 counter16 = ((counter+mod) & 0x0000FFFF);
					uint32 counter32 = (counter+mod) + 0xFF000000;

					if ((counter+mod) < 0xFF00){
						indice16 = MSB2(counter16);
						f.Write(reinterpret_cast<char *>(&indice16),2);
						polySize += 2;
					}else{
						//wxLogMessage("Counter above limit: %i", counter);
						indice32 = reverse_endian<uint32>(counter32);
						f.Write(reinterpret_cast<char *>(&indice32), 4);
						polySize += 4;
					}
				}
				polycount++;
			}
			tnBatches++;
		}
	}

	off_t = -4-polySize;
	f.SeekO(off_t, wxFromCurrent);
	u32 = reverse_endian<uint32>(polySize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	f.SeekO(0, wxFromEnd);

#ifdef _DEBUG
	wxLogMessage("WMO Poly Count: %i, Stored Polys: %i, Polysize: %i, Stored Polysize: %i",polycount, gpolys, polySize,gpolys * sizeof(PolyChunk16) );
	wxLogMessage("WMO nGroups: %i, nBatchs: %i",m->nGroups, tnBatches);
#endif

	fileLen += polySize;
	// ========


	// --
	// The PTAG chunk associates tags with polygons. In this case, it identifies which surface is assigned to each polygon. 
	// The first number in each pair is a 0-based index into the most recent POLS chunk, and the second is a 0-based 
	// index into the TAGS chunk.

	// NOTE: Every PTAG type needs a seperate PTAG call!

	TagCounter = 0;
	PartCounter = 0;
	counter = 0;
	int32 ptagSize;

	// Parts PolyTag
	f.Write("PTAG", 4);
	ptagSize = 4;
	counter=0;
	u32 = reverse_endian<uint32>(ptagSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	fileLen += 8;
	f.Write("PART", 4);
	for (int g=0;g<m->nGroups;g++) {
		for (int b=0; b<m->groups[g].nBatches; b++) {
			WMOBatch *batch = &m->groups[g].batches[b];

			for (unsigned int k=0; k<batch->indexCount; k+=3) {
				uint16 indice16;
				uint32 indice32;

				uint16 counter16 = (counter & 0x0000FFFF);
				uint32 counter32 = counter + 0xFF000000;

				if (counter < 0xFF00){
					indice16 = MSB2(counter16);
					f.Write(reinterpret_cast<char *>(&indice16),2);
					ptagSize += 2;
				}else{
					indice32 = reverse_endian<uint32>(counter32);
					f.Write(reinterpret_cast<char *>(&indice32), 4);
					ptagSize += 4;
				}

				u16 = MSB2(TagCounter);
				f.Write(reinterpret_cast<char *>(&u16), 2);
				ptagSize += 2;
				counter++;
			}
		}
		TagCounter++;
		PartCounter++;
	}
	fileLen += ptagSize;

	off_t = -4-ptagSize;
	f.SeekO(off_t, wxFromCurrent);
	u32 = reverse_endian<uint32>(ptagSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	f.SeekO(0, wxFromEnd);


	// Surface PolyTag
	counter = 0;
	SurfCounter = 0;
	f.Write("PTAG", 4);
	ptagSize = 4;
	u32 = reverse_endian<uint32>(ptagSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	fileLen += 8;
	f.Write("SURF", 4);
	for (int g=0;g<m->nGroups;g++) {
		for (int b=0; b<m->groups[g].nBatches; b++) {
			WMOBatch *batch = &m->groups[g].batches[b];

			for (uint32 k=0; k<batch->indexCount; k+=3) {
				uint16 indice16;
				uint32 indice32;

				uint16 counter16 = (counter & 0x0000FFFF);
				uint32 counter32 = counter + 0xFF000000;

				if (counter < 0xFF00){
					indice16 = MSB2(counter16);
					f.Write(reinterpret_cast<char *>(&indice16),2);
					ptagSize += 2;
				}else{
					indice32 = reverse_endian<uint32>(counter32);
					f.Write(reinterpret_cast<char *>(&indice32), 4);
					ptagSize += 4;
				}

				int surfID = TagCounter + 0;
				for (int16 x=0;x<m->nTextures;x++){
					wxString target = texarray[m->mat[batch->texture].tex];
					if (surfarray[x] == target){
						surfID = TagCounter + x;
						break;
					}
				}

				u16 = MSB2(surfID);
				f.Write(reinterpret_cast<char *>(&u16), 2);
				ptagSize += 2;
				counter++;
			}
			SurfCounter++;
		}
	}
	fileLen += ptagSize;

	off_t = -4-ptagSize;
	f.SeekO(off_t, wxFromCurrent);
	u32 = reverse_endian<uint32>(ptagSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	f.SeekO(0, wxFromEnd);


	// ===================================================
	//VMPA		// Vertex Map Parameters, Always Preceeds a VMAP & VMAD. 4bytes: Size, then Num Vars (2) * 4 bytes.
				// UV Sub Type: 0-Linear, 1-Subpatched, 2-Linear Corners, 3-Linear Edges, 4-Across Discontinuous Edges.
				// Sketch Color: 0-12; 6-Default Gray
	// ===================================================
	f.Write("VMPA", 4);
	u32 = reverse_endian<uint32>(8);	// We got 2 Paramaters, * 4 Bytes.
	f.Write(reinterpret_cast<char *>(&u32), 4);
	u32 = 0;							// Linear UV Sub Type
	f.Write(reinterpret_cast<char *>(&u32), 4);
	u32 = reverse_endian<uint32>(6);	// Default Gray
	f.Write(reinterpret_cast<char *>(&u32), 4);
	fileLen += 16;


	// ===================================================
	// Discontinuous Vertex Mapping
	// ===================================================

	int32 vmadSize = 0;
	f.Write("VMAD", 4);
	u32 = reverse_endian<uint32>(vmadSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	fileLen += 8;
	// UV Data
	f.Write("TXUV", 4);
	dimension = 2;
	dimension = MSB2(dimension);
	f.Write(reinterpret_cast<char *>(&dimension), 2);
	f.Write("Texture", 7);
	ub = 0;
	f.Write(reinterpret_cast<char *>(&ub), 1);
	vmadSize += 14;

	counter = 0;
	for (int g=0;g<m->nGroups;g++) {
		for (int b=0; b<m->groups[g].nBatches; b++)
		{
			WMOBatch *batch = &m->groups[g].batches[b];

			for (size_t k=0, b=0; k<batch->indexCount; k++,b++) {
				int a = m->groups[g].indices[batch->indexStart + k];
				uint16 indice16;
				uint32 indice32;

				uint16 counter16 = (counter & 0x0000FFFF);
				uint32 counter32 = counter + 0xFF000000;

				if (counter < 0xFF00){
					indice16 = MSB2(counter16);
					f.Write(reinterpret_cast<char *>(&indice16),2);
					vmadSize += 2;
				}else{
					indice32 = reverse_endian<uint32>(counter32);
					f.Write(reinterpret_cast<char *>(&indice32), 4);
					vmadSize += 4;
				}

				if (((counter/3) & 0x0000FFFF) < 0xFF00){
					indice16 = MSB2(((counter/3) & 0x0000FFFF));
					f.Write(reinterpret_cast<char *>(&indice16),2);
					vmadSize += 2;
				}else{
					indice32 = reverse_endian<uint32>((counter/3) + 0xFF000000);
					f.Write(reinterpret_cast<char *>(&indice32), 4);
					vmadSize += 4;
				}
				f32 = reverse_endian<float>(m->groups[g].texcoords[a].x);
				f.Write(reinterpret_cast<char *>(&f32), 4);
				f32 = reverse_endian<float>(1 - m->groups[g].texcoords[a].y);
				f.Write(reinterpret_cast<char *>(&f32), 4);
				counter++;
				vmadSize += 8;
			}
		}
	}
	fileLen += vmadSize;
	off_t = -4-vmadSize;
	f.SeekO(off_t, wxFromCurrent);
	u32 = reverse_endian<uint32>(vmadSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	f.SeekO(0, wxFromEnd);
	

	// ===================================================
	// Texture File List
	// ===================================================

	// Export Texture Files
	// There is currently a bug where the improper textures are being assigned the wrong filenames.
	// Once the proper textures can be assigned the proper filenames, then this will be re-installed.
	/*
	for (size_t x=0;x<m->textures.size();x++){
		SaveTexture2(m->textures[x], wxString(fn, wxConvUTF8).BeforeLast('\\'),"LWO","tga");
	}
	*/

	// Texture Data
	uint32 surfaceCounter = PartCounter;
	uint32 ClipCount = 0;
	for (int g=0;g<m->nGroups;g++) {
		for (int b=0; b<m->groups[g].nBatches; b++)
		{
			WMOBatch *batch = &m->groups[g].batches[b];
			WMOMaterial *mat = &m->mat[batch->texture];

			int clipSize = 0;
			f.Write("CLIP", 4);
			u32 = reverse_endian<uint32>(0);
			f.Write(reinterpret_cast<char *>(&u32), 4);
			fileLen += 8;

			u32 = reverse_endian<uint32>(++surfaceCounter);
			f.Write(reinterpret_cast<char *>(&u32), 4);
			f.Write("STIL", 4);
			clipSize += 8;

			glBindTexture(GL_TEXTURE_2D, mat->tex);
			//wxLogMessage("Opening %s so we can save it...",texarray[mat->tex]);
			wxString FilePath = wxString(fn, wxConvUTF8).BeforeLast('\\');
			wxString texName = texarray[mat->tex];
			wxString texPath = texName.BeforeLast('\\');
			texName = texName.AfterLast('\\');

			wxString sTexName = "";
			if (modelExport_LW_PreserveDir == true){
				sTexName += "Images/";
			}
			if (modelExport_PreserveDir == true){
				sTexName += texPath;
				sTexName << "\\";
				sTexName.Replace("\\","/");
			}
			sTexName += texName;

			sTexName << _T(".tga") << '\0';

			if (fmod((float)sTexName.length(), 2.0f) > 0)
				sTexName.Append(_T('\0'));

			u16 = MSB2(sTexName.length());
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f.Write(sTexName.data(), sTexName.length());
			clipSize += (2+sTexName.length());

			// update the chunks length
			off_t = -4-clipSize;
			f.SeekO(off_t, wxFromCurrent);
			u32 = reverse_endian<uint32>(clipSize);
			f.Write(reinterpret_cast<char *>(&u32), 4);
			f.SeekO(0, wxFromEnd);

			// save texture to file
			wxString texFilename;
			texFilename = FilePath;
			texFilename += '\\';
			texFilename += texName;

			if (modelExport_LW_PreserveDir == true){
				MakeDirs(FilePath,"Images");

				texFilename.Empty();
				texFilename = FilePath << "\\" << "Images" << "\\" << texName;
			}

			if (modelExport_PreserveDir == true){
				wxString Path;
				Path << texFilename.BeforeLast('\\');

				MakeDirs(Path,texPath);

				texFilename.Empty();
				texFilename = Path << '\\' << texPath << '\\' << texName;
			}
			texFilename << (".tga");
			wxLogMessage("Saving Image: %s",texFilename);

			// setup texture
			SaveTexture(texFilename);

			fileLen += clipSize;
		}
	}


	// ===================================================
	// Surface Data
	// ===================================================

	wxString surfName;
	surfaceCounter = PartCounter;
	for (int g=0;g<m->nGroups;g++) {
		for (int b=0; b<m->groups[g].nBatches; b++)
		{
			WMOBatch *batch = &m->groups[g].batches[b];
			WMOMaterial *mat = &m->mat[batch->texture];

			uint32 surfaceDefSize = 0;
			f.Write("SURF", 4);
			u32 = reverse_endian<uint32>(surfaceDefSize);
			f.Write(reinterpret_cast<char *>(&u32), 4);
			fileLen += 8;

			// Surface name
			//surfName = wxString::Format(_T("Material_%03i"),mat->tex);
			++surfaceCounter;

			surfName = texarray[mat->tex].AfterLast('\\');			
			surfName.Append(_T('\0'));
			if (fmod((float)surfName.length(), 2.0f) > 0)
				surfName.Append(_T('\0'));

			surfName.Append(_T('\0')); // Source: ""
			surfName.Append(_T('\0')); // Even out the source
			f.Write(surfName.data(), (int)surfName.length());
			
			surfaceDefSize += surfName.length();

			// Surface Attributes
			// COLOR, size 4, bytes 2
			f.Write("COLR", 4);
			u16 = 14; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			
			// value
			f32 = reverse_endian<float>(0.5);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f32 = reverse_endian<float>(0.5);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f32 = reverse_endian<float>(0.5);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			
			surfaceDefSize += 20;

			// LUMI
			f.Write("LUMI", 4);
			u16 = 6; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 0;
			f32 = reverse_endian<float>(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);


			surfaceDefSize += 12;

			// DIFF
			f.Write("DIFF", 4);
			u16 = 6; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 1;
			f32 = reverse_endian<float>(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);

			surfaceDefSize += 12;


			// SPEC
			f.Write("SPEC", 4);
			u16 = 6; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 0;
			f32 = reverse_endian<float>(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);

			surfaceDefSize += 12;

			// REFL
			f.Write("REFL", 4);
			u16 = 6; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			ub = '\0';
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);

			surfaceDefSize += 12;

			// TRAN
			f.Write("TRAN", 4);
			u16 = 6; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			ub = '\0';
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);

			surfaceDefSize += 12;

			// GLOSSINESS
			f.Write("GLOS", 4);
			u16 = 6; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			
			// Value
			// Set to 20%, because that seems right.
			f32 = 0.2;
			f32 = reverse_endian<float>(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			
			surfaceDefSize += 12;

			// SMAN (Smoothing)
			f.Write("SMAN", 4);
			u16 = 4; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			// Smoothing is done in radiens. PI = 180 degree smoothing.
			f32 = PI;
			f32 = reverse_endian<float>(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			surfaceDefSize += 10;


			// RFOP
			f.Write("RFOP", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 1;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);

			surfaceDefSize += 8;

			// TROP
			f.Write("TROP", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 1;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);

			surfaceDefSize += 8;

			// SIDE
			f.Write("SIDE", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 1;
			// If double-sided...
			if (mat->flags & WMO_MATERIAL_CULL){
				u16 = 3;
			}
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);

			surfaceDefSize += 8;

			// --
			// BLOK
			uint16 blokSize = 0;
			f.Write("BLOK", 4);
			f.Write(reinterpret_cast<char *>(&blokSize), 2);
			surfaceDefSize += 6;

			// IMAP
			f.Write("IMAP", 4);
			u16 = 50-8; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 0x80;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;

			// CHAN
			f.Write("CHAN", 4);
			u16 = 4; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f.Write("COLR", 4);
			blokSize += 10;

			// OPAC
			f.Write("OPAC", 4);
			u16 = 8; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 1.0;
			f32 = reverse_endian<float>(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 14;

			// ENAB
			f.Write("ENAB", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 1;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;

			// NEGA
			f.Write("NEGA", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 0;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;
/*
			// AXIS
			f.Write("AXIS", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 1;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;
*/
			// TMAP
			f.Write("TMAP", 4);
			u16 = 98; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 6;

			// CNTR
			f.Write("CNTR", 4);
			u16 = 14; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 0.0;
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 20;

			// SIZE
			f.Write("SIZE", 4);
			u16 = 14; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 1.0;
			f32 = reverse_endian<float>(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 20;

			// ROTA
			f.Write("ROTA", 4);
			u16 = 14; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 0.0;
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 20;

			// FALL
			f.Write("FALL", 4);
			u16 = 16; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 0.0;
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 22;

			// OREF
			f.Write("OREF", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;

			// CSYS
			f.Write("CSYS", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;

			// end TMAP

			// PROJ
			f.Write("PROJ", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 5;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;

			// AXIS
			f.Write("AXIS", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 2;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;

			// IMAG
			f.Write("IMAG", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = surfaceCounter;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;

			// WRAP
			f.Write("WRAP", 4);
			u16 = 4; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 1;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 1;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 10;

			// WRPW
			f.Write("WRPW", 4);
			u16 = 6; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 1;
			f32 = reverse_endian<float>(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 12;

			// WRPH
			f.Write("WRPH", 4);
			u16 = 6; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 1;
			f32 = reverse_endian<float>(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 12;

			// VMAP
			f.Write("VMAP", 4);
			u16 = 8; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			wxString t = _T("Texture");
			t.Append(_T('\0'));
			f.Write(t.data(), t.length());
			blokSize += 14;

			// AAST
			f.Write("AAST", 4);
			u16 = 6; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 1;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 1;
			f32 = reverse_endian<float>(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			blokSize += 12;

			// PIXB
			f.Write("PIXB", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 1;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;

			// Fix Blok Size
			surfaceDefSize += blokSize;
			off_t = -2-blokSize;
			f.SeekO(off_t, wxFromCurrent);
			u16 = MSB2(blokSize);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f.SeekO(0, wxFromEnd);
			// ================

			// CMNT
			f.Write("CMNT", 4);
			wxString cmnt = texarray[mat->tex];
			cmnt.Append(_T('\0'));
			if (fmod((float)cmnt.length(), 2.0f) > 0)
				cmnt.Append(_T('\0'));
			u16 = cmnt.length(); // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f.Write(cmnt.data(), cmnt.length());
			surfaceDefSize += 6 + cmnt.length();

			f.Write("VERS", 4);
			u16 = 4;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 950;
			f32 = reverse_endian<int32>(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			surfaceDefSize += 10;
				
			// Fix Surface Size
			fileLen += surfaceDefSize;
			off_t = -4-surfaceDefSize;
			f.SeekO(off_t, wxFromCurrent);
			u32 = reverse_endian<uint32>(surfaceDefSize);
			f.Write(reinterpret_cast<char *>(&u32), 4);
			f.SeekO(0, wxFromEnd);
		}
	}
	// ================


	f.SeekO(4, wxFromStart);
	u32 = reverse_endian<uint32>(fileLen);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	f.SeekO(0, wxFromEnd);

	f.Close();

	// Cleanup, Isle 3!
	wxDELETEA(texarray);

	// Export Lights & Doodads
	if ((modelExport_LW_ExportDoodads == true)||(modelExport_LW_ExportLights == true)){
		ExportWMOObjectstoLWO(m,fn);
	}
}



// -----------------------------------------
// New Lightwave Stuff
//
// Under construction, only visible/usable while in Debug Mode.
// -----------------------------------------


// Seperated out the Writing function, so we don't have to write it all out every time we want to export something.
// Should probably do something similar with the other exporting functions as well...
bool WriteLWObject(wxString filename, wxString Tags[], int LayerNum, LWLayer Layers[]){

	// LightWave object files use the IFF syntax described in the EA-IFF85 document. Data is stored in a collection of chunks. 
	// Each chunk begins with a 4-byte chunk ID and the size of the chunk in bytes, and this is followed by the chunk contents.


	/* LWO Model Format, as layed out in offical LWO2 files.( I Hex Edited to find most of this information from files I made/saved in Lightwave. -Kjasi)
	FORM	// Format Declaration
	LWO2	// Declares this is the Lightwave Object 2 file format. LWOB is the first format. Doesn't have a lot of the cool stuff LWO2 has...

	TAGS	// Used for various Strings
		Sketch Color Names
		Part Names
		Surface Names
	LAYR		// Specifies the start of a new layer. Probably best to only be on one...
		PNTS		// Points listing & Block Section
			BBOX		// Bounding Box. It's optional, but will probably help.
			VMPA		// Vertex Map Parameters, Always Preceeds a VMAP & VMAD. 4bytes: Size (2 * 4 bytes).
						// UV Sub Type: 0-Linear, 1-Subpatched, 2-Linear Corners, 3-Linear Edges, 4-Across Discontinuous Edges.
						// Sketch Color: 0-12; 6-Default Gray
				VMAP		// Vector Map Section. Always Preceeds the following:
					SPOT	// Aboslute Morph Maps. Used only while modeling. Ignore.
					TXUV	// Defines UV Vector Map. Best not to use these unless the data has no Discontinuous UVs.
					PICK	// Point Selection Sets (2 bytes, then Set Name, then data. (Don't know what kind)
					MORF	// Relative Morph Maps. These are used for non-boned mesh animation.
					RGB		// Point Color Map, no Alpha. Note the space at end of the group!
					RGBA	// Same as above, but with an alpha channel.
					WGHT	// Weight Map. Used to give bones limited areas to effect, or can be used for point-by-point maps for various surfacing tricks.

		POLS		// Declares Polygon section. Next 4 bytes = Number of Polys
			FACE		// The actual Polygons. The maximum number of vertices is 1023 per poly!
				PTAG		// The Poly Tags for this Poly. These usually reference items in the TAGS group.
					COLR	// The Sketch Color Name
					PART	// The Part Name
					SURF	// The Surface Name
				VMPA		// Discontinuous Vertex Map Parameters (See the one in the Points section for details)
					VMAD		// Discontinuous Vector Map Section. Best if used only for UV Maps. Difference between VMAP & VMAD: VMAPs are connected to points, while VMADs are connected to Polys.
						APSL	// Adaptive Pixel Subdivision Level. Only needed for sub-patched models, so just ignore it for our outputs.
						TXUV	// Defines UV Vector Map
			PTCH	// Cat-mull Clarke Patches. Don't need this, but it mirror's FACE's sub-chunks.
			SUBD	// Subdivision Patches. Same as above.
			MBAL	// Metaballs. Don't bother...
			BONE	// Line segments representing the object's skeleton. These are converted to bones for deformation during rendering.

	CLIP (for each Image)
		STIL	// 2 bytes, size of string, followed by image name.extention
		FLAG	// Flags. 2 bytes, size of chunk. Not sure what the flag values are.
		AMOD	// 2 bytes: What's changed, 2 bytes: value. 2-Alphas: 0-Enabled, 1-Disabled, 2-Alpha Only. AMOD is omitted if value is 0.
	XREF	// Calls an instance of a CLIP. We'll avoid this for now.

	SURF	// Starts the surface's data. Not sure about the 4 bytes after it...
		// Until BLOK, this just sets the default values
		COLR	// Color
		LUMI	// Luminosity
		DIFF	// Diffusion
		SPEC	// Specularity
		REFL	// Reflections
		TRAN	// Transparancy
		TRNL	// Translucency
		RIND	// Refractive Index
		BUMP	// Bump Amount
		GLOS	// Glossiness
		GVAL	// Glow
		SHRP	// Diffuse Sharpness

		SMAN	// Smoothing Amount

		RFOP	// Reflection Options: 0-Backdrop Only (default), 1-Raytracing + Backdrop, 2 - Spherical Map, 3 - Raytracing + Spherical Map
		TROP	// Same as RFOP, but for Refraction.
		SIDE	// Is it Double-Sided?
		NVSK	// Exclude from VStack

		CMNT // Surface Comment, but I don't seem to be able to get it to show up in LW... 2bytes: Size. Simple Text line for this surface. Make sure it doesn't end on an odd byte! VERS must be 931 or 950!
		VERS // Version Compatibility mode, including what it's compatible with. 2 bytes (int16, value 4), 4 bytes (int32, value is 850 for LW8.5 Compatability, 931 for LW9.3.1, and 950 for Default)

		BLOK	// First Blok. Bloks hold Surface texture information!
			IMAP	// Declares that this surface texture is an image map.
				CHAN COLR	// Declares that the image map will be applied to the color channel. (Color has a Texture!)
					OPAC	// Opacity of Layer
					ENAB	// Is the layer enabled?
					NEGA	// Is it inverted?
					TMAP	// Texture Map details
						CNTR	// Position
						SIZE	// Scale
						ROTA	// Rotation
						FALL	// Falloff
						OREF	// Object Reference
						CSYS	// Coordinate System: 0-Object's Coordinates, 1-World's Coordinates

						// Image Maps
						PROJ	// Image Projection Mode: 0-Planar (references AXIS), 1-Cylindrical, 2-Spherical, 3-Cubic, 4-Front Projection, 5-UV (IDed in VMAP chunk)
						AXIS	// The axis the image uses: 0-X, 1-Y, or 2-Z;
						IMAG	// The image to use: Use CLIP Index
						WRAP	// Wrapping Mode: 0-Reset, 1-Repeat, 2-Mirror, 3-Edge
						WRPW	// Wrap Count Width (Used for Cylindrical & Spherical projections)
						WRPH	// Wrap Count Height
						VMAP	// Name of the UV Map to use, should PROJ be set to 5!
						AAST	// Antialiasing Strength
						PIXB	// Pixel Blending

		// Node Information
		// We can probably skip this for now. Later, it would be cool to mess with it, but for now, it'll be automatically generated once the file is opened in LW.

		NODS	// Node Block & Size
			NROT
			NLOC
			NZOM
			NSTA	// Activate Nodes
			NVER
			NNDS
			NSRV
				Surface
			NTAG
			NRNM
				Surface
			NNME
				Surface
			NCRD
			NMOD
			NDTA
			NPRW
			NCOM
			NCON

	*/

	// Check to see if we have the proper data to generate this file.
	// At the very least, we need some point data...
	if (!Layers[0].Points[0]){
		wxLogMessage("Error: No Layer Data. Unable to write object file.");
		return false;
	}


   	// -----------------------------------------
	// Initial Variables
	// -----------------------------------------

	// File Length
	unsigned int fileLen = 0;

	// Other Declares
	int off_t;
	uint16 dimension;

	// Needed Numbers
	int TagCount = Tags->size();

	// Open Model File
	wxString file = wxString(filename, wxConvUTF8);

	if (modelExport_LW_PreserveDir == true){
		wxString Path, Name;

		Path << file.BeforeLast('\\');
		Name << file.AfterLast('\\');

		MakeDirs(Path,"Objects");

		file.Empty();
		file << Path << "\\Objects\\" << Name;
	}
	if (modelExport_PreserveDir == true){
		wxString Path1, Path2, Name;
		Path1 << file.BeforeLast('\\');
		Name << file.AfterLast('\\');
		Path2 << wxString(filename).BeforeLast('\\');

		MakeDirs(Path1,Path2);

		file.Empty();
		file << Path1 << '\\' << Path2 << '\\' << Name;
	}

	wxFFileOutputStream f(file, wxT("w+b"));

	if (!f.IsOk()) {
		wxLogMessage(_T("Error: Unable to open file '%s'. Could not export model."), file);
		return false;
	}

	// ===================================================
	// FORM		// Format Declaration
	//
	// Always exempt from the length of the file!
	// ===================================================
	f.Write("FORM", 4);
	f.Write(reinterpret_cast<char *>(&fileLen), 4);

	// ===================================================
	// LWO2
	//
	// Declares this is the Lightwave Object 2 file format.
	// LWOB is the first format. It doesn't have a lot of the cool stuff LWO2 has...
	// ===================================================
	f.Write("LWO2", 4);
	fileLen += 4;

	// ===================================================
	// TAGS
	//
	// Used for various Strings. Known string types, in order:
	//		Sketch Color Names
	//		Part Names
	//		Surface Names
	// ===================================================
	f.Write("TAGS", 4);
	uint32 tagsSize = 0;
	f.Write(reinterpret_cast<char *>(&tagsSize), 4);
	fileLen += 8;

	// For each Tag...
	for (int i=0; i<TagCount; i++){
		wxString tagName = Tags[i];

		tagName.Append(_T('\0'));
		if (fmod((float)tagName.length(), 2.0f) > 0)
			tagName.Append(_T('\0'));
		f.Write(tagName.data(), tagName.length());
		tagsSize += tagName.length();
	}
	// Correct TAGS Length
	off_t = -4-tagsSize;
	f.SeekO(off_t, wxFromCurrent);
	u32 = reverse_endian<uint32>(tagsSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	f.SeekO(0, wxFromEnd);

	fileLen += tagsSize;


	// -------------------------------------------------
	// Generate our Layers
	//
	// Point, Poly & Vertex Map data will be nested in
	// our layers.
	// -------------------------------------------------
	for (int l=0;l<LayerNum;l++){
		// Define a Layer & It's data
		uint16 LayerNameSize = Layers[l].Name.Len();
		uint16 LayerSize = 18+LayerNameSize;
		f.Write("LAYR", 4);
		u32 = reverse_endian<uint32>(LayerSize);
		f.Write(reinterpret_cast<char *>(&u32), 4);
		fileLen += 8;

		// Layer Number
		u16 = MSB2(l);
		f.Write(reinterpret_cast<char *>(&u16), 2);
		// Flags
		f.Write(reinterpret_cast<char *>(0), 2);
		// Pivot
		f.Write(reinterpret_cast<char *>(0), 4);
		f.Write(reinterpret_cast<char *>(0), 4);
		f.Write(reinterpret_cast<char *>(0), 4);
		// Name
		if (LayerNameSize>0){
			f.Write(Layers[l].Name, LayerNameSize);
		}
		// Parent
		f.Write(reinterpret_cast<char *>(0), 2);
		fileLen += LayerSize;


		// -------------------------------------------------
		// Points Chunk
		//
		// There will be new Point Chunk for every Layer, so if we go
		// beyond 1 Layer, this should be nested.
		// -----------------------------------------

		uint32 pointsSize = 0;
		f.Write("PNTS", 4);
		u32 = reverse_endian<uint32>(pointsSize);
		f.Write(reinterpret_cast<char *>(&u32), 4);
		fileLen += 8;

		// Writes the point data
		for (int i=0; i<Layers[l].PointCount; i++) {
			Vec3D vert;
			vert.x = reverse_endian<float>(Layers[l].Points[i].x);
			vert.y = reverse_endian<float>(Layers[l].Points[i].z);
			vert.z = reverse_endian<float>(Layers[l].Points[i].y);

			f.Write(reinterpret_cast<char *>(&vert.x), 4);
			f.Write(reinterpret_cast<char *>(&vert.y), 4);
			f.Write(reinterpret_cast<char *>(&vert.z), 4);

			pointsSize += 12;
			free (vert);
		}
		// Corrects the filesize...
		fileLen += pointsSize;
		off_t = -4-pointsSize;
		f.SeekO(off_t, wxFromCurrent);
		u32 = reverse_endian<uint32>(pointsSize);
		f.Write(reinterpret_cast<char *>(&u32), 4);
		f.SeekO(0, wxFromEnd);


		// -----------------------------------------
		// Point Vertex Maps
		// UV, Weights, Vertex Color Maps, etc.
		// -----------------------------------------
		f.Write("VMAP", 4);
		uint32 vmapSize = 0;
		u32 = reverse_endian<uint32>(vmapSize);
		f.Write(reinterpret_cast<char *>(&u32), 4);
		fileLen += 8;

	/*
		// UV Data
		f.Write("TXUV", 4);
		dimension = 2;
		dimension = MSB2(dimension);
		f.Write(reinterpret_cast<char *>(&dimension), 2);
		vmapSize += 6;

		wxString UVMapName;
		UVMapName << filename << '\0';
		if (fmod((float)UVMapName.length(), 2.0f) > 0)
			UVMapName.Append(_T('\0'));
		f.Write(UVMapName.data(), UVMapName.length());
		vmapSize += UVMapName.length();
		*/
		/*
		for (int g=0; g<m->nGroups; g++) {
			for (int b=0; b<m->groups[g].nBatches; b++)
			{
				WMOBatch *batch = &m->groups[g].batches[b];
				for(int ii=0;ii<batch->indexCount;ii++)
				{
					int a = m->groups[g].indices[batch->indexStart + ii];
					u16 = MSB2(counter);
					f.Write(reinterpret_cast<char *>(&u16), 2);
					f32 = reverse_endian<float>(m->groups[g].texcoords[a].x);
					f.Write(reinterpret_cast<char *>(&f32), 4);
					f32 = reverse_endian<float>(1 - m->groups[g].texcoords[a].y);
					f.Write(reinterpret_cast<char *>(&f32), 4);
					counter++;
					if (counter == 256)
						counter = 0;
					vmapSize += 10;
				}
			}
		}
		*/
	/*
		fileLen += vmapSize;

		off_t = -4-vmapSize;
		f.SeekO(off_t, wxFromCurrent);
		u32 = reverse_endian<uint32>(vmapSize);
		f.Write(reinterpret_cast<char *>(&u32), 4);
		f.SeekO(0, wxFromEnd);

		// -----------------------------------------
		// Polygon Chunk
		// -----------------------------------------
		if (PolyCount > 0){
			f.Write("POLS", 4);
			uint32 polySize = 4;
			u32 = MSB4(polySize);
			f.Write(reinterpret_cast<char *>(&u32), 4);
			fileLen += 8; // FACE is handled in the PolySize
			f.Write("FACE", 4);

			PolyChunk32 swapped;
			int size = 2;
			for (int p=0;p<PolyCount;p++){
				swapped.numVerts = MSB2(Polys[p].numVerts);
				if (Polys[p].indice[0] < 0xFF){
					swapped.indice[0] =  MSB2((Polys[p].indice[0] & 0x0000FFFF));
					size += 2;
				}else{
					swapped.indice[0] =  MSB4((Polys[p].indice[0] + 0xFF000000));
					size += 4;
				}
				if (Polys[p].indice[2] < 0xFF){
					swapped.indice[1] =  MSB2((Polys[p].indice[2] & 0x0000FFFF));
					size += 2;
				}else{
					swapped.indice[1] =  MSB4((Polys[p].indice[2] + 0xFF000000));
					size += 4;
				}
				if (Polys[p].indice[1] < 0xFF){
					swapped.indice[2] =  MSB2((Polys[p].indice[1] & 0x0000FFFF));
					size += 2;
				}else{
					swapped.indice[2] =  MSB4((Polys[p].indice[1] + 0xFF000000));
					size += 4;
				}

				polySize += size;
				f.Write(reinterpret_cast<char *>(&swapped), size);
				wxLogMessage(_T("Writing polygon %i..."), p);
			}
			off_t = -4-polySize;
			f.SeekO(off_t, wxFromCurrent);
			u32 = MSB4(polySize);
			f.Write(reinterpret_cast<char *>(&u32), 4);
			f.SeekO(0, wxFromEnd);
			
			fileLen += polySize;
		}
	*/
	}

	// Correct File Length
	f.SeekO(4, wxFromStart);
	u32 = reverse_endian<uint32>(fileLen);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	f.SeekO(0, wxFromEnd);

	f.Close();

	// If we've gotten this far, then the file is good!
	return true;
}



// No longer writes data to a LWO file. Instead, it collects the data, and send it to a seperate function that writes the actual file.
void ExportWMOtoLWO2(WMO *m, const char *fn)
{
	wxString filename(fn, wxConvUTF8);

	wxString Tags[1];
	int TagCount = 0;
	int LayerNum = 1;
	LWLayer Layers[1];






	WriteLWObject(filename, Tags, LayerNum, Layers);


/*
	uint32 counter=0;
	uint32 TagCounter=0;
	uint16 PartCounter=0;
	uint16 SurfCounter=0;
	unsigned int numVerts = 0;
	unsigned int numGroups = 0;


	// Texture Name Array
	// Find a Match for mat->tex and place it into the Texture Name Array.
	wxString *texarray = new wxString[m->nTextures+1];
	for (int g=0;g<m->nGroups;g++) {
		for (int b=0; b<m->groups[g].nBatches; b++)
		{
			WMOBatch *batch = &m->groups[g].batches[b];
			WMOMaterial *mat = &m->mat[batch->texture];
			wxString outname(fn, wxConvUTF8);

			bool nomatch = true;
			for (int t=0;t<=m->nTextures; t++) {
				if (t == mat->tex) {
					texarray[mat->tex] = m->textures[t-1];
					texarray[mat->tex] = texarray[mat->tex].BeforeLast('.');
					nomatch = false;
					break;
				}
			}
			if (nomatch == true){
				texarray[mat->tex] = outname << wxString::Format(_T("_Material_%03i"), mat->tex);
			}
		}
	}

	// Part Names
	for (int g=0;g<m->nGroups;g++) {
		wxString partName = m->groups[g].name;

		partName.Append(_T('\0'));
		if (fmod((float)partName.length(), 2.0f) > 0)
			partName.Append(_T('\0'));
		f.Write(partName.data(), partName.length());
		tagsSize += partName.length();
	}

	// Surface Names
	wxString *surfarray = new wxString[m->nTextures+1];
	for (unsigned int t=0;t<m->nTextures;t++){
		wxString matName = wxString(m->textures[t]).BeforeLast('.');

		matName.Append(_T('\0'));
		if (fmod((float)matName.length(), 2.0f) > 0)
			matName.Append(_T('\0'));
		f.Write(matName.data(), matName.length());
		tagsSize += matName.length();
		surfarray[t] = wxString(m->textures[t]).BeforeLast('.');
	}

	off_t = -4-tagsSize;
	f.SeekO(off_t, wxFromCurrent);
	u32 = MSB4(tagsSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	f.SeekO(0, wxFromEnd);
	fileLen += tagsSize;



	// ===================================================
	// LAYR
	//
	// Specifies the start of a new layer. Each layer has it's own Point & Poly
	// chunk, which tells it what data is on what layer. It's probably best
	// to only have 1 layer for now.
	// ===================================================
	f.Write("LAYR", 4);
	u32 = MSB4(18);
	fileLen += 8;
	f.Write(reinterpret_cast<char *>(&u32), 4);
	ub = 0;
	for(int i=0; i<18; i++) {
		f.Write(reinterpret_cast<char *>(&ub), 1);
	}
	fileLen += 18;
	// ================


	// ===================================================
	// POINTS CHUNK, this is the vertice data
	// The PNTS chunk contains triples of floating-point numbers, the coordinates of a list of points. The numbers are written 
	// as IEEE 32-bit floats in network byte order. The IEEE float format is the standard bit pattern used by almost all CPUs 
	// and corresponds to the internal representation of the C language float type. In other words, this isn't some bizarre 
	// proprietary encoding. You can process these using simple fread and fwrite calls (but don't forget to correct the byte 
	// order if necessary).
	// ===================================================
	uint32 pointsSize = 0;
	f.Write("PNTS", 4);
	u32 = MSB4(pointsSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	fileLen += 8;

	unsigned int bindice = 0;
	unsigned int gverts = 0;

	// output all the vertice data
	for (int g=0; g<m->nGroups; g++) {
		gverts += m->groups[g].nVertices;
		for (int b=0; b<m->groups[g].nBatches; b++)	{
			WMOBatch *batch = &m->groups[g].batches[b];

			bindice += batch->indexCount;
			for(int ii=0;ii<batch->indexCount;ii++){
				int a = m->groups[g].indices[batch->indexStart + ii];
				Vec3D vert;
				vert.x = reverse_endian<float>(m->groups[g].vertices[a].x);
				vert.y = reverse_endian<float>(m->groups[g].vertices[a].z);
				vert.z = reverse_endian<float>(m->groups[g].vertices[a].y);
				f.Write(reinterpret_cast<char *>(&vert.x), 4);
				f.Write(reinterpret_cast<char *>(&vert.y), 4);
				f.Write(reinterpret_cast<char *>(&vert.z), 4);
				pointsSize += 12;

				numVerts++;
			}
			numGroups++;
		}
	}
#ifdef _DEBUG
	wxLogMessage("WMO Point Count: %i, Stored Indices: %i, Stored Verticies: %i",numVerts, bindice, gverts);
#endif


	fileLen += pointsSize;
	off_t = -4-pointsSize;
	f.SeekO(off_t, wxFromCurrent);
	u32 = MSB4(pointsSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	f.SeekO(0, wxFromEnd);
	// ================


	// --
	// The bounding box for the layer, just so that readers don't have to scan the PNTS chunk to find the extents.
	f.Write("BBOX", 4);
	u32 = MSB4(24);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	Vec3D vert;
	vert.x = MSB4(m->v1.x);
	vert.z = MSB4(m->v1.y);
	vert.y = MSB4(m->v1.z);
	f.Write(reinterpret_cast<char *>(&vert.x), 4);
	f.Write(reinterpret_cast<char *>(&vert.y), 4);
	f.Write(reinterpret_cast<char *>(&vert.z), 4);
	vert.x = MSB4(m->v2.x);
	vert.z = MSB4(m->v2.y);
	vert.y = MSB4(m->v2.z);
	f.Write(reinterpret_cast<char *>(&vert.x), 4);
	f.Write(reinterpret_cast<char *>(&vert.y), 4);
	f.Write(reinterpret_cast<char *>(&vert.z), 4);
	fileLen += 32;



	// Removed Point Vertex Mapping for WMOs.
	// UV Map now generated by Discontinuous Vertex Mapping, down in the Poly section.

	uint32 vmapSize = 0;

	//Vertex Mapping
	f.Write("VMAP", 4);
	u32 = MSB4(vmapSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	fileLen += 8;
	// UV Data
	f.Write("TXUV", 4);
	dimension = MSB2(2);
	f.Write(reinterpret_cast<char *>(&dimension), 2);
	f.Write("Texture", 7);
	ub = 0;
	f.Write(reinterpret_cast<char *>(&ub), 1);
	vmapSize += 14;

	// u16, f32, f32
	for (int i=0; i<m->nGroups; i++) {
		for (int j=0; j<m->groups[i].nBatches; j++)
		{
			WMOBatch *batch = &m->groups[i].batches[j];
			for(int ii=0;ii<batch->indexCount;ii++)
			{
				int a = m->groups[i].indices[batch->indexStart + ii];


				uint16 indice16;
				uint32 indice32;

				uint16 counter16 = (counter & 0x0000FFFF);
				uint32 counter32 = counter + 0xFF000000;

				if (counter < 0xFF00){
					indice16 = MSB2(counter16);
					f.Write(reinterpret_cast<char *>(&indice16),2);
					vmapSize += 2;
				}else{
					indice32 = MSB4(counter32);
					f.Write(reinterpret_cast<char *>(&indice32), 4);
					vmapSize += 4;
				}

				f32 = MSB4(m->groups[i].texcoords[a].x);
				f.Write(reinterpret_cast<char *>(&f32), 4);
				f32 = MSB4(1 - m->groups[i].texcoords[a].y);
				f.Write(reinterpret_cast<char *>(&f32), 4);
				vmapSize += 8;
				counter++;
			}
		}
	}
	fileLen += vmapSize;

	off_t = -4-vmapSize;
	f.SeekO(off_t, wxFromCurrent);
	u32 = MSB4(vmapSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	f.SeekO(0, wxFromEnd);




	// --
	// POLYGON CHUNK
	// The POLS chunk contains a list of polygons. A "polygon" in this context is anything that can be described using an 
	// ordered list of vertices. A POLS of type FACE contains ordinary polygons, but the POLS type can also be CURV, 
	// PTCH, MBAL or BONE, for example.
	//
	// The high 6 bits of the vertex count for each polygon are reserved for flags, which in effect limits the number of 
	// vertices per polygon to 1023. Don't forget to mask the high bits when reading the vertex count. The flags are 
	// currently only defined for CURVs.
	// 
	// The point indexes following the vertex count refer to the points defined in the most recent PNTS chunk. Each index 
	// can be a 2-byte or a 4-byte integer. If the high order (first) byte of the index is not 0xFF, the index is 2 bytes long. 
	// This allows values up to 65279 to be stored in 2 bytes. If the high order byte is 0xFF, the index is 4 bytes long and 
	// its value is in the low three bytes (index & 0x00FFFFFF). The maximum value for 4-byte indexes is 16,777,215 (224 - 1). 
	// Objects with more than 224 vertices can be stored using multiple pairs of PNTS and POLS chunks.
	// 
	// The cube has 6 square faces each defined by 4 vertices. LightWave polygons are single-sided by default 
	// (double-sidedness is a possible surface property). The vertices are listed in clockwise order as viewed from the 
	// visible side, starting with a convex vertex. (The normal is defined as the cross product of the first and last edges.)

	f.Write("POLS", 4);
	uint32 polySize = 4;
	u32 = MSB4(polySize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	fileLen += 8; // FACE is handled in the PolySize
	f.Write("FACE", 4);

	counter = 0;
	unsigned long polycount = 0;
	uint32 gpolys = 0;
	unsigned int tnBatches = 0;
	
	for (int g=0;g<m->nGroups;g++) {
		gpolys += m->groups[g].nTriangles;
		for (int b=0; b<m->groups[g].nBatches; b++){
			WMOBatch *batch = &m->groups[g].batches[b];
			for (unsigned int k=0; k<batch->indexCount; k+=3) {
				uint16 nverts;

				// Write the number of Verts
				nverts = MSB2(3);
				f.Write(reinterpret_cast<char *>(&nverts),2);
				polySize += 2;

				for (int x=0;x<3;x++,counter++){
					//wxLogMessage("Batch %i, index %i, x=%i",b,k,x);
					uint16 indice16;
					uint32 indice32;

					int mod = 0;
					if (x==1){
						mod = 1;
					}else if (x==2){
						mod = -1;
					}

					uint16 counter16 = ((counter+mod) & 0x0000FFFF);
					uint32 counter32 = (counter+mod) + 0xFF000000;

					if ((counter+mod) < 0xFF00){
						indice16 = MSB2(counter16);
						f.Write(reinterpret_cast<char *>(&indice16), 2);
						polySize += 2;
					}else{
						//wxLogMessage("Counter above limit: %i", counter);
						indice32 = MSB4(counter32);
						f.Write(reinterpret_cast<char *>(&indice32), 4);
						polySize += 4;
					}
				}
				polycount++;
			}
			tnBatches++;
		}
	}

	off_t = -4-polySize;
	f.SeekO(off_t, wxFromCurrent);
	u32 = MSB4(polySize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	f.SeekO(0, wxFromEnd);

#ifdef _DEBUG
	wxLogMessage("WMO Poly Count: %i, Stored Polys: %i, Polysize: %i, Stored Polysize: %i",polycount, gpolys, polySize,gpolys * sizeof(PolyChunk16) );
	wxLogMessage("WMO nGroups: %i, nBatchs: %i",m->nGroups, tnBatches);
#endif

	fileLen += polySize;
	// ========


	// --
	// The PTAG chunk associates tags with polygons. In this case, it identifies which surface is assigned to each polygon. 
	// The first number in each pair is a 0-based index into the most recent POLS chunk, and the second is a 0-based 
	// index into the TAGS chunk.

	// NOTE: Every PTAG type needs a seperate PTAG call!

	TagCounter = 0;
	PartCounter = 0;
	counter=0;
	int32 ptagSize;

	// Parts PolyTag
	f.Write("PTAG", 4);
	ptagSize = 4;
	counter=0;
	u32 = MSB4(ptagSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	fileLen += 8;
	f.Write("PART", 4);
	for (int g=0;g<m->nGroups;g++) {
		for (int b=0; b<m->groups[g].nBatches; b++) {
			WMOBatch *batch = &m->groups[g].batches[b];

			for (unsigned int k=0; k<batch->indexCount; k+=3) {
				uint16 indice16;
				uint32 indice32;

				uint16 counter16 = (counter & 0x0000FFFF);
				uint32 counter32 = counter + 0xFF000000;

				if (counter < 0xFF00){
					indice16 = MSB2(counter16);
					f.Write(reinterpret_cast<char *>(&indice16),2);
					ptagSize += 2;
				}else{
					indice32 = MSB4(counter32);
					f.Write(reinterpret_cast<char *>(&indice32), 4);
					ptagSize += 4;
				}

				u16 = MSB2(TagCounter);
				f.Write(reinterpret_cast<char *>(&u16), 2);
				ptagSize += 2;
				counter++;
			}
		}
		TagCounter++;
		PartCounter++;
	}
	fileLen += ptagSize;

	off_t = -4-ptagSize;
	f.SeekO(off_t, wxFromCurrent);
	u32 = MSB4(ptagSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	f.SeekO(0, wxFromEnd);


	// Surface PolyTag
	counter = 0;
	SurfCounter = 0;
	f.Write("PTAG", 4);
	ptagSize = 4;
	u32 = MSB4(ptagSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	fileLen += 8;
	f.Write("SURF", 4);
	for (int g=0;g<m->nGroups;g++) {
		for (int b=0; b<m->groups[g].nBatches; b++) {
			WMOBatch *batch = &m->groups[g].batches[b];

			for (unsigned int k=0; k<batch->indexCount; k+=3) {
				uint16 indice16;
				uint32 indice32;

				uint16 counter16 = (counter & 0x0000FFFF);
				uint32 counter32 = counter + 0xFF000000;

				if (counter < 0xFF00){
					indice16 = MSB2(counter16);
					f.Write(reinterpret_cast<char *>(&indice16),2);
					ptagSize += 2;
				}else{
					indice32 = MSB4(counter32);
					f.Write(reinterpret_cast<char *>(&indice32), 4);
					ptagSize += 4;
				}

				int surfID = TagCounter + 0;
				for (int x=0;x<m->nTextures;x++){
					wxString target = texarray[m->mat[batch->texture].tex];
					if (surfarray[x] == target){
						surfID = TagCounter + x;
						break;
					}
				}

				u16 = MSB2(surfID);
				f.Write(reinterpret_cast<char *>(&u16), 2);
				ptagSize += 2;
				counter++;
			}
			SurfCounter++;
		}
	}
	fileLen += ptagSize;

	off_t = -4-ptagSize;
	f.SeekO(off_t, wxFromCurrent);
	u32 = MSB4(ptagSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	f.SeekO(0, wxFromEnd);


	// ===================================================
	//VMPA		// Vertex Map Parameters, Always Preceeds a VMAP & VMAD. 4bytes: Size, then Num Vars (2) * 4 bytes.
				// UV Sub Type: 0-Linear, 1-Subpatched, 2-Linear Corners, 3-Linear Edges, 4-Across Discontinuous Edges.
				// Sketch Color: 0-12; 6-Default Gray
	// ===================================================
	f.Write("VMPA", 4);
	u32 = MSB4(8);	// We got 2 Paramaters, * 4 Bytes.
	f.Write(reinterpret_cast<char *>(&u32), 4);
	u32 = 0;							// Linear UV Sub Type
	f.Write(reinterpret_cast<char *>(&u32), 4);
	u32 = MSB4(6);	// Default Gray
	f.Write(reinterpret_cast<char *>(&u32), 4);
	fileLen += 16;


	// ===================================================
	// Discontinuous Vertex Mapping
	// ===================================================

	int32 vmadSize = 0;
	f.Write("VMAD", 4);
	u32 = MSB4(vmadSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	fileLen += 8;
	// UV Data
	f.Write("TXUV", 4);
	dimension = 2;
	dimension = MSB2(dimension);
	f.Write(reinterpret_cast<char *>(&dimension), 2);
	f.Write("Texture", 7);
	ub = 0;
	f.Write(reinterpret_cast<char *>(&ub), 1);
	vmadSize += 14;

	counter = 0;
	// u16, u16, f32, f32
	for (int g=0;g<m->nGroups;g++) {
		for (int b=0; b<m->groups[g].nBatches; b++)
		{
			WMOBatch *batch = &m->groups[g].batches[b];

			for (size_t k=0, b=0; k<batch->indexCount; k++,b++) {
				int a = m->groups[g].indices[batch->indexStart + k];
				uint16 indice16;
				uint32 indice32;

				uint16 counter16 = (counter & 0x0000FFFF);
				uint32 counter32 = counter + 0xFF000000;

				if (counter < 0xFF00){
					indice16 = MSB2(counter16);
					f.Write(reinterpret_cast<char *>(&indice16),2);
					vmadSize += 2;
				}else{
					indice32 = MSB4(counter32);
					f.Write(reinterpret_cast<char *>(&indice32), 4);
					vmadSize += 4;
				}

				if (((counter/3) & 0x0000FFFF) < 0xFF00){
					indice16 = MSB2(((counter/3) & 0x0000FFFF));
					f.Write(reinterpret_cast<char *>(&indice16),2);
					vmadSize += 2;
				}else{
					indice32 = MSB4((counter/3) + 0xFF000000);
					f.Write(reinterpret_cast<char *>(&indice32), 4);
					vmadSize += 4;
				}
			//	u16 = MSB2((uint16)(counter/3));
			//	f.Write(reinterpret_cast<char *>(&u16), 2);
				f32 = MSB4(m->groups[g].texcoords[a].x);
				f.Write(reinterpret_cast<char *>(&f32), 4);
				f32 = MSB4(1 - m->groups[g].texcoords[a].y);
				f.Write(reinterpret_cast<char *>(&f32), 4);
				counter++;
				vmadSize += 8;
			}
		}
	}
	fileLen += vmadSize;
	off_t = -4-vmadSize;
	f.SeekO(off_t, wxFromCurrent);
	u32 = MSB4(vmadSize);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	f.SeekO(0, wxFromEnd);



	// ===================================================
	// Texture File List
	// ===================================================
	uint32 surfaceCounter = PartCounter;
	uint32 ClipCount = 0;
	for (int g=0;g<m->nGroups;g++) {
		for (int b=0; b<m->groups[g].nBatches; b++)
		{
			WMOBatch *batch = &m->groups[g].batches[b];
			WMOMaterial *mat = &m->mat[batch->texture];

			int clipSize = 0;
			f.Write("CLIP", 4);
			u32 = MSB4(0);
			f.Write(reinterpret_cast<char *>(&u32), 4);
			fileLen += 8;

			u32 = MSB4(++surfaceCounter);
			f.Write(reinterpret_cast<char *>(&u32), 4);
			f.Write("STIL", 4);
			clipSize += 8;

			glBindTexture(GL_TEXTURE_2D, mat->tex);
			//wxLogMessage("Opening %s so we can save it...",texarray[mat->tex]);
			wxString FilePath = wxString(fn, wxConvUTF8).BeforeLast('\\');
			wxString texName = texarray[mat->tex];
			wxString texPath = texName.BeforeLast('\\');
			texName = texName.AfterLast('\\');

			wxString sTexName = "";
			if (modelExport_LW_PreserveDir == true){
				sTexName += "Images/";
			}
			if (modelExport_PreserveDir == true){
				sTexName += texPath;
				sTexName << "\\";
				sTexName.Replace("\\","/");
			}
			sTexName += texName;

			sTexName << _T(".tga") << '\0';

			if (fmod((float)sTexName.length(), 2.0f) > 0)
				sTexName.Append(_T('\0'));

			u16 = MSB2(sTexName.length());
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f.Write(sTexName.data(), sTexName.length());
			clipSize += (2+sTexName.length());

			// update the chunks length
			off_t = -4-clipSize;
			f.SeekO(off_t, wxFromCurrent);
			u32 = MSB4(clipSize);
			f.Write(reinterpret_cast<char *>(&u32), 4);
			f.SeekO(0, wxFromEnd);

			// save texture to file
			wxString texFilename;
			texFilename = FilePath;
			texFilename += '\\';
			texFilename += texName;

			if (modelExport_LW_PreserveDir == true){
				MakeDirs(FilePath,"Images");

				texFilename.Empty();
				texFilename = FilePath << "\\" << "Images" << "\\" << texName;
			}

			if (modelExport_PreserveDir == true){
				wxString Path;
				Path << texFilename.BeforeLast('\\');

				MakeDirs(Path,texPath);

				texFilename.Empty();
				texFilename = Path << '\\' << texPath << '\\' << texName;
			}
			texFilename << (".tga");
			wxLogMessage("Saving Image: %s",texFilename);

			// setup texture
			SaveTexture(texFilename);

			fileLen += clipSize;
		}
	}
	// ================


	// --
	wxString surfName;
	surfaceCounter = PartCounter;
	for (int g=0;g<m->nGroups;g++) {
		for (int b=0; b<m->groups[g].nBatches; b++)
		{
			WMOBatch *batch = &m->groups[g].batches[b];
			WMOMaterial *mat = &m->mat[batch->texture];

			uint32 surfaceDefSize = 0;
			f.Write("SURF", 4);
			u32 = MSB4(surfaceDefSize);
			f.Write(reinterpret_cast<char *>(&u32), 4);
			fileLen += 8;

			// Surface name
			//surfName = wxString::Format(_T("Material_%03i"),mat->tex);
			++surfaceCounter;

			surfName = texarray[mat->tex];			
			surfName.Append(_T('\0'));
			if (fmod((float)surfName.length(), 2.0f) > 0)
				surfName.Append(_T('\0'));

			surfName.Append(_T('\0')); // Source: ""
			surfName.Append(_T('\0')); // Even out the source
			f.Write(surfName.data(), (int)surfName.length());
			
			surfaceDefSize += surfName.length();

			// Surface Attributes
			// COLOR, size 4, bytes 2
			f.Write("COLR", 4);
			u16 = 14; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			
			// value
			f32 = MSB4(0.5);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f32 = MSB4(0.5);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f32 = MSB4(0.5);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			
			surfaceDefSize += 20;

			// LUMI
			f.Write("LUMI", 4);
			u16 = 6; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 0;
			f32 = MSB4(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);


			surfaceDefSize += 12;

			// DIFF
			f.Write("DIFF", 4);
			u16 = 6; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 1;
			f32 = MSB4(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);

			surfaceDefSize += 12;


			// SPEC
			f.Write("SPEC", 4);
			u16 = 6; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 0;
			f32 = MSB4(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);

			surfaceDefSize += 12;

			// REFL
			f.Write("REFL", 4);
			u16 = 6; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			ub = '\0';
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);

			surfaceDefSize += 12;

			// TRAN
			f.Write("TRAN", 4);
			u16 = 6; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			ub = '\0';
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);
			f.Write(reinterpret_cast<char *>(&ub), 1);

			surfaceDefSize += 12;

			// GLOSSINESS
			f.Write("GLOS", 4);
			u16 = 6; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			
			// Value
			// Set to 20%, because that seems right.
			f32 = 0.2;
			f32 = MSB4(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			
			surfaceDefSize += 12;

			// SMAN (Smoothing)
			f.Write("SMAN", 4);
			u16 = 4; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			// Smoothing is done in radiens. PI = 180 degree smoothing.
			f32 = PI;
			f32 = MSB4(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			surfaceDefSize += 10;


			// RFOP
			f.Write("RFOP", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 1;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);

			surfaceDefSize += 8;

			// TROP
			f.Write("TROP", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 1;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);

			surfaceDefSize += 8;

			// SIDE
			f.Write("SIDE", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 1;
			// If double-sided...
			if (mat->flags & WMO_MATERIAL_CULL){
				u16 = 3;
			}
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);

			surfaceDefSize += 8;

			// --
			// BLOK
			uint16 blokSize = 0;
			f.Write("BLOK", 4);
			f.Write(reinterpret_cast<char *>(&blokSize), 2);
			surfaceDefSize += 6;

			// IMAP
			f.Write("IMAP", 4);
			u16 = 50-8; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 0x80;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;

			// CHAN
			f.Write("CHAN", 4);
			u16 = 4; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f.Write("COLR", 4);
			blokSize += 10;

			// OPAC
			f.Write("OPAC", 4);
			u16 = 8; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 1.0;
			f32 = MSB4(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 14;

			// ENAB
			f.Write("ENAB", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 1;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;

			// NEGA
			f.Write("NEGA", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 0;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;
			*/
/*
			// AXIS
			f.Write("AXIS", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 1;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;
*/
/*
			// TMAP
			f.Write("TMAP", 4);
			u16 = 98; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 6;

			// CNTR
			f.Write("CNTR", 4);
			u16 = 14; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 0.0;
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 20;

			// SIZE
			f.Write("SIZE", 4);
			u16 = 14; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 1.0;
			f32 = MSB4(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 20;

			// ROTA
			f.Write("ROTA", 4);
			u16 = 14; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 0.0;
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 20;

			// FALL
			f.Write("FALL", 4);
			u16 = 16; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 0.0;
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 22;

			// OREF
			f.Write("OREF", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;

			// CSYS
			f.Write("CSYS", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 0;
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;

			// end TMAP

			// PROJ
			f.Write("PROJ", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 5;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;

			// AXIS
			f.Write("AXIS", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 2;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;

			// IMAG
			f.Write("IMAG", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = surfaceCounter;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;

			// WRAP
			f.Write("WRAP", 4);
			u16 = 4; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 1;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 1;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 10;

			// WRPW
			f.Write("WRPW", 4);
			u16 = 6; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 1;
			f32 = MSB4(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 12;

			// WRPH
			f.Write("WRPH", 4);
			u16 = 6; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 1;
			f32 = MSB4(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			u16 = 0;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 12;

			// VMAP
			f.Write("VMAP", 4);
			u16 = 8; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			wxString t = _T("Texture");
			t.Append(_T('\0'));
			f.Write(t.data(), t.length());
			blokSize += 14;

			// AAST
			f.Write("AAST", 4);
			u16 = 6; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 1;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 1;
			f32 = MSB4(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			blokSize += 12;

			// PIXB
			f.Write("PIXB", 4);
			u16 = 2; // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			u16 = 1;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			blokSize += 8;

			// Fix Blok Size
			surfaceDefSize += blokSize;
			off_t = -2-blokSize;
			f.SeekO(off_t, wxFromCurrent);
			u16 = MSB2(blokSize);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f.SeekO(0, wxFromEnd);
			// ================

			// CMNT
			f.Write("CMNT", 4);
			wxString cmnt = texarray[mat->tex];
			cmnt.Append(_T('\0'));
			if (fmod((float)cmnt.length(), 2.0f) > 0)
				cmnt.Append(_T('\0'));
			u16 = cmnt.length(); // size
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f.Write(cmnt.data(), cmnt.length());
			surfaceDefSize += 6 + cmnt.length();

			f.Write("VERS", 4);
			u16 = 4;
			u16 = MSB2(u16);
			f.Write(reinterpret_cast<char *>(&u16), 2);
			f32 = 950;
			f32 = MSB4(f32);
			f.Write(reinterpret_cast<char *>(&f32), 4);
			surfaceDefSize += 10;
			
			// Fix Surface Size
			fileLen += surfaceDefSize;
			off_t = -4-surfaceDefSize;
			f.SeekO(off_t, wxFromCurrent);
			u32 = MSB4(surfaceDefSize);
			f.Write(reinterpret_cast<char *>(&u32), 4);
			f.SeekO(0, wxFromEnd);
		}
	}
	// ================


	f.SeekO(4, wxFromStart);
	u32 = MSB4(fileLen);
	f.Write(reinterpret_cast<char *>(&u32), 4);
	f.SeekO(0, wxFromEnd);

	f.Close();

	// Cleanup, Isle 3!
	wxDELETEA(texarray);

	// Export Lights & Doodads
	//if ((modelExport_LW_ExportDoodads == true)||(modelExport_LW_ExportLights == true)){
	//	ExportWMOObjectstoLWO(m,fn);
	//}
	*/

}