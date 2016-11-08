#ifndef OBJECT_H_INCLUDED
#define OBJECT_H_INCLUDED

#include <utility>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdio>
#include <gl/glut.h>

using namespace std;

typedef struct		
{
	float x;
	float y;
	float z;
} VERTEX;

class Object {		

public:

	typedef std::vector<VERTEX>::iterator viter;				
	typedef std::vector<std::pair<float,float>>::iterator titer;
	typedef std::vector<VERTEX>::iterator niter;
	typedef std::vector<int>::iterator fiter;

	Object(viter vbeg,viter vend,titer tbeg,titer tend,niter nbeg,niter nend,fiter fbeg,fiter fend,int r,int c,std::string mat)
	:vertexs(vbeg,vend),texcoords(tbeg,tend),normals(nbeg,nend),faces(fbeg,fend),row(r),col(c),material(mat){}
	Object(){}

	std::vector<VERTEX> vertexs;
	std::vector< std::pair<float,float> > texcoords;
	std::vector<VERTEX> normals;
	std::vector<int> faces;
	std::string material;
	int row,col;
};

class Material {		

public:
	Material(float *a,float *d,float *s,float *e,unsigned m)
	{
		ambient[0] = *a;
		ambient[1] = *(a+1);
		ambient[2] = *(a+2);
		ambient[3] = 0;
		diffuse[0] = *d;
		diffuse[1] = *(d+1);
		diffuse[2] = *(d+2);
		diffuse[3] = 0;
		specular[0] = *s;
		specular[1] = *(s+1);
		specular[2] = *(s+2);
		specular[3] = 0;
		emission[0] = *e;
		emission[1] = *(e+1);
		emission[2] = *(e+2);
		emission[3] = 0;
		map = m;
	}
	Material()
	{
		ambient[0] = 0;
		ambient[1] = 0;
		ambient[2] = 0;
		ambient[3] = 0;
		diffuse[0] = 1.0;
		diffuse[1] = 1.0;
		diffuse[2] = 1.0;
		diffuse[3] = 0;
		specular[0] = 1.0;
		specular[1] = 1.0;
		specular[2] = 1.0;
		specular[3] = 0;
		emission[0] = 0;
		emission[1] = 0;
		emission[2] = 0;
		emission[3] = 0;
		map = 0;
	}
	float ambient[4];
	float diffuse[4];
	float specular[4];
	float emission[4];
	unsigned map;
};


inline int power_of_two(int n)		
{
	if(n<=0) return 0;
	return (n&(n-1))==0;		
}

inline bool startswith(string &a,string &b)		
{
	if(a.size()<b.size())
	{
		return false;
	}
	return a.substr(0,b.size())==b;		
}

GLuint load_texture(const char *file_name)
{
	FILE	*pFile;
	GLubyte *pixels = NULL;
	GLint	 width, height, total_bytes;
	GLuint	 last_texture_ID, texture_ID = 0;

	if((pFile=fopen(file_name, "rb"))==NULL)		
	{
		cout<<"Read texture error"<<endl;
		return 0;
	}

	fseek(pFile,18,SEEK_SET);				
	//int fseek(FILE *stream, long offset, int fromwhere); 
	fread(&width,sizeof(width),1,pFile);	
	//size_t fread ( void *buffer, size_t size, size_t count, FILE *stream);

	fread(&height,sizeof(height),1,pFile);	
	fseek(pFile,54,SEEK_SET);			

	total_bytes = (width*3+(4-width*3%4)%4)*height;

	if((pixels=(GLubyte *)malloc(total_bytes))==NULL)
	{
		fclose(pFile);
		return 0;
	}

	if(fread(pixels,total_bytes,1,pFile)<=0)
	{
		free(pixels);
		fclose(pFile);
		return 0;
	}

	{
		GLint max;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE,&max);	
		if(!power_of_two(width)||!power_of_two(height)||width>max||height>max)	
		{
			const GLint new_width = 1024;
			const GLint new_height = 1024;
			GLint new_total_bytes;
			GLubyte *new_pixels = NULL;

			new_total_bytes = (new_width*3+(4-new_width*3%4)%4)*new_height;
			new_pixels = (GLubyte *)malloc(new_total_bytes);
			if(new_pixels==NULL)
			{
				free(pixels);
				fclose(pFile);
				return 0;
			}

			//gluScaleImage() 
			gluScaleImage(GL_RGB,width,height,GL_UNSIGNED_BYTE,pixels,new_width,new_height,GL_UNSIGNED_BYTE,new_pixels);

			free(pixels);
			pixels = new_pixels;
			width = new_width;
			height = new_height;
		}
	}

	glGenTextures(1,&texture_ID);
	if(texture_ID==0)
	{
		free(pixels);
		fclose(pFile);
		return 0;
	}

	glGetIntegerv(GL_TEXTURE_BINDING_2D,(GLint *)&last_texture_ID);
	glBindTexture(GL_TEXTURE_2D,texture_ID);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,GL_BGR_EXT,GL_UNSIGNED_BYTE,pixels);
	//glBindTexture(GL_TEXTURE_2D,last_texture_ID);

	free(pixels);
	fclose(pFile);
	return texture_ID;
}

void ReadMtl(string &cd,string mtlfile,map<string,Material> &mat)
{
	ifstream in;
	string line,word,ptname,ntname,fname;
	unsigned map;
	bool hasmap = false;
	float ambient[3] = {0.0,0.0,0.0};
	float diffuse[3] = {0.0,0.0,0.0};
	float specular[3] = {0.0,0.0,0.0};
	float emission[3] = {0.0,0.0,0.0};

	in.open((cd+"\\"+mtlfile).c_str());
	if(!in)
	{
		cout<<"Read mtl error !"<<endl;
		return;
	}
	while(getline(in,line))
	{
		if(line.size()==0||line[0]=='#') continue;	
		istringstream is(line);

		is>>word;
		if(word=="newmtl")
		{
			is>>ntname;
			if(!ptname.empty())
			{
				if(hasmap)
				{
					mat.insert(make_pair(ptname,Material(ambient,diffuse,specular,emission,map)));
				}
				else
				{
					mat.insert(make_pair(ptname,Material(ambient,diffuse,specular,emission,0)));
				}
			}
			ptname = ntname;
			hasmap = false;
		}
		else if(word=="Ka")                                             //ambient info
		{
			is>>ambient[0]>>ambient[1]>>ambient[2];	
		}
		else if(word=="Kd")                                             //diffuse info
		{
			is>>diffuse[0]>>diffuse[1]>>diffuse[2];
		}
		else if(word=="Ks")                                             //specular info
		{
			is>>specular[0]>>specular[1]>>specular[2];
		}
		else if(word=="Ke")                                             //emission info
		{
			is>>emission[0]>>emission[1]>>emission[2];
		}
		else if(word=="map_Ka")                                         //read bmp
		{
			is>>fname;			
			map = load_texture((cd+"\\"+fname).c_str());

			hasmap = true;
		}
	}
	if(!ntname.empty())
	{
		if(hasmap)
		{
			mat.insert(make_pair(ptname,Material(ambient,diffuse,specular,emission,map)));
		}
		else
		{
			mat.insert(make_pair(ptname,Material(ambient,diffuse,specular,emission,0)));
		}
	}
	in.close();
}

void ReadObj(string &cd,string file,map<string,Object> &m,set<string> &n,map<string,Material> &matname)
{
	ifstream in;
	vector<VERTEX> vertexs;
	vector< pair<float,float> > texcoords;
	vector<VERTEX> normals;
	vector<int> faces;
	int row = 0,col = 0;
	string line,word,goname,mtlname;
	char Buffer[MAX_PATH];

	if(file.find(":")!=string::npos)
	{
		cd = string(file.begin(),file.begin()+file.rfind("\\"));
	}
	else if(startswith(file,string(".\\")))
	{
		GetCurrentDirectoryA(MAX_PATH,Buffer);
		cd = Buffer+string(file.begin()+1,file.begin()+file.rfind("\\"));
	}
	else if(startswith(file,string("..\\")))
	{
		GetCurrentDirectoryA(MAX_PATH,Buffer);
		cd = Buffer;
		cd = string(cd.begin(),cd.begin()+cd.rfind("\\"));
		cd = cd + string(file.begin()+2,file.begin()+file.rfind("\\"));
		cout<<cd<<endl;
	}
	else
	{
		GetCurrentDirectoryA(MAX_PATH,Buffer);
		if(file.rfind("\\")!=string::npos)
		{
			cd = Buffer+string("\\")+string(file.begin(),file.begin()+file.rfind("\\"));
		}
		else
		{
			cd = Buffer;
		}
	}

	in.open(file.c_str());
	if(!in)
	{
		cout<<"Read obj error !"<<endl;
		exit(0);
	}
	while(getline(in,line))
	{
		if(line.size()==0||line[0]=='#') continue;	
		istringstream is(line);
		is>>word;
		if(word=="v")                               //geometric vertices
		{
			VERTEX p;
			is>>p.x>>p.y>>p.z;
			vertexs.push_back(p);	
		}
		else if(word=="vt")                         //texture vertices
		{
			pair<float,float> p;
			is>>p.first>>p.second;
			texcoords.push_back(p);
		}
		else if(word=="vn")                         //vertex normals
        {
			VERTEX p;
			is>>p.x>>p.y>>p.z;
			normals.push_back(p);
		}
		
		else if(word=="f")                          //face
		{
			int r = 0,c = 0;
			while(is>>word)
			{
				c = count(word.begin(),word.end(),'/');		
				if(c==0)
				{
					faces.push_back(atoi(word.c_str()));	
				}
				else if(c==1)
				{
					faces.push_back(atoi(string(word.begin(),word.begin()+word.find("/")).c_str()));
					faces.push_back(atoi(string(word.begin()+word.find("/")+1,word.end()).c_str()));
				}
				else if(c==2)
				{
					int a = word.find("/");
					int b = word.find("/",a+1);
					faces.push_back(atoi(string(word.begin(),word.begin()+a).c_str()));
					faces.push_back(atoi(string(word.begin()+a+1,word.begin()+b).c_str()));
					faces.push_back(atoi(string(word.begin()+b+1,word.end()).c_str()));
				}
				++r;
			}
			row = r;
			col = c+1;
		}
		else if(word=="mtllib")             //read mtl file
		{
			is>>word;
			ReadMtl(cd,word,matname);
		}
	}
	if(!goname.empty()&&!faces.empty())
	{
		Object obj(vertexs.begin(),vertexs.end(),texcoords.begin(),texcoords.end(),normals.begin(),normals.end(),faces.begin(),faces.end(),row,col,mtlname);
		while(n.count(goname)!=0)
		{
			goname.push_back('0');
		}
		m.insert(make_pair(goname,obj));
		n.insert(goname);
		faces.clear();
	}
	in.close();
}

#endif