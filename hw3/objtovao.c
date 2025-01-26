#include "CSCIx239.h"
#include <ctype.h>

typedef struct {
    float v[3];
    float t[2];
    float n[3];
} Vertex_t;

//
//  Return true if CR or LF
//
static int CRLF(char ch)
{
   return ch == '\r' || ch == '\n';
}

//
//  Read line from file
//    Returns pointer to line or NULL on EOF
//
static int linelen=0;    //  Length of line
static char* line=NULL;  //  Internal storage for line
static char* readline(FILE* f)
{
   char ch;  //  Character read
   int k=0;  //  Character count
   while ((ch = fgetc(f)) != EOF)
   {
      //  Allocate more memory for long strings
      if (k>=linelen)
      {
         linelen += 8192;
         line = (char*)realloc(line,linelen);
         if (!line) Fatal("Out of memory in readline\n");
      }
      //  End of Line
      if (CRLF(ch))
      {
         // Eat extra CR or LF characters (if any)
         while ((ch = fgetc(f)) != EOF)
           if (!CRLF(ch)) break;
         //  Stick back the overrun
         if (ch != EOF) ungetc(ch,f);
         //  Bail
         break;
      }
      //  Pad character to line
      else
         line[k++] = ch;
   }
   //  Terminate line if anything was read
   if (k>0) line[k] = 0;
   //  Return pointer to line or NULL on EOF
   return k>0 ? line : NULL;
}

//
//  Read to next non-whitespace word
//  Note that this destroys line in the process
//
static char* getword(char** line)
{
   //  Skip leading whitespace
   while (**line && isspace(**line))
      (*line)++;
   if (!**line) return NULL;
   //  Start of word
   char* word = *line;
   //  Read until next whitespace
   while (**line && !isspace(**line))
      (*line)++;
   //  Mark end of word if not NULL
   if (**line)
   {
      **line = 0;
      (*line)++;
   }
   return word;
}

static int countWords(char* line) {
    int count=0;
    while (getword(&line)) count++;
    return count;
};

static void countBufferSizes(FILE* f, int* nv, int* ni) {
    int v=0; // Number of vertices
    int i=0; // Number of indices

    char* line;
    while ((line = readline(f))) {
        // printf("%s",line);
        //  Vertex coordinates (always 3)
        if (line[0]=='v' && line[1]==' ')
            v++;
        //  Read and draw facets
        else if (line[0]=='f') {
            i += countWords(line) - 1;
        }
    }
    // Set counts
    *nv = v;
    *ni = i;
}

int ObjToVao(char* file) {
	// int  Nv,Nn,Nt;  //  Number of vertex, normal and textures
	// int  Mv,Mn,Mt;  //  Maximum vertex, normal and textures
	// float* V;       //  Array of vertexes
	// float* N;       //  Array of normals
	// float* T;       //  Array if textures coordinates
	// char*  line;    //  Line pointer
	// char*  str;     //  String pointer

    //  Open file
    FILE* f = fopen(file,"r");
    if (!f) Fatal("Cannot open file %s\n",file);

    // Count how many vertices and indices are in file
    int Nv=0, Ni=0;
    countBufferSizes(f,&Nv,&Ni);

    printf("%d vertices, %d indices\n", Nv, Ni);

//     //  Start new displaylist
//     int list = glGenLists(1);
//     glNewList(list,GL_COMPILE);
//     //  Push attributes for textures
//     glPushAttrib(GL_ENABLE_BIT|GL_TEXTURE_BIT);

//    //  Read vertexes and facets
//    V  = N  = T  = NULL;
//    Nv = Nn = Nt = 0;
//    Mv = Mn = Mt = 0;
//    while ((line = readline(f)))
//    {
//       //  Vertex coordinates (always 3)
//       if (line[0]=='v' && line[1]==' ')
//          readcoord(line+2,3,&V,&Nv,&Mv);
//       //  Normal coordinates (always 3)
//       else if (line[0]=='v' && line[1] == 'n')
//          readcoord(line+2,3,&N,&Nn,&Mn);
//       //  Texture coordinates (always 2)
//       else if (line[0]=='v' && line[1] == 't')
//          readcoord(line+2,2,&T,&Nt,&Mt);
//       //  Read and draw facets
//       else if (line[0]=='f')
//       {
//          line++;
//          //  Read Vertex/Texture/Normal triplets
//          glBegin(GL_POLYGON);
//          while ((str = getword(&line)))
//          {
//             int Kv,Kt,Kn;
//             //  Try Vertex/Texture/Normal triplet
//             if (sscanf(str,"%d/%d/%d",&Kv,&Kt,&Kn)==3)
//             {
//                if (Kv<0 || Kv>Nv/3) Fatal("Vertex %d out of range 1-%d\n",Kv,Nv/3);
//                if (Kn<0 || Kn>Nn/3) Fatal("Normal %d out of range 1-%d\n",Kn,Nn/3);
//                if (Kt<0 || Kt>Nt/2) Fatal("Texture %d out of range 1-%d\n",Kt,Nt/2);
//             }
//             //  Try Vertex//Normal pairs
//             else if (sscanf(str,"%d//%d",&Kv,&Kn)==2)
//             {
//                if (Kv<0 || Kv>Nv/3) Fatal("Vertex %d out of range 1-%d\n",Kv,Nv/3);
//                if (Kn<0 || Kn>Nn/3) Fatal("Normal %d out of range 1-%d\n",Kn,Nn/3);
//                Kt = 0;
//             }
//             //  Try Vertex index
//             else if (sscanf(str,"%d",&Kv)==1)
//             {
//                if (Kv<0 || Kv>Nv/3) Fatal("Vertex %d out of range 1-%d\n",Kv,Nv/3);
//                Kn = 0;
//                Kt = 0;
//             }
//             //  This is an error
//             else
//                Fatal("Invalid facet %s\n",str);
//             //  Draw vectors
//             if (Kt) glTexCoord2fv(T+2*(Kt-1));
//             if (Kn) glNormal3fv(N+3*(Kn-1));
//             if (Kv) glVertex3fv(V+3*(Kv-1));
//          }
//          glEnd();
//       }
//       //  Use material
//       else if ((str = readstr(line,"usemtl")))
//          SetMaterial(str);
//       //  Load materials
//       else if ((str = readstr(line,"mtllib")))
//          LoadMaterial(str);
//       //  Skip this line
//    }
//    fclose(f);
//    //  Pop attributes (textures)
//    glPopAttrib();
//    glEndList();

//    //  Free materials
//    for (int k=0;k<Nmtl;k++)
//       free(mtl[k].name);
//    free(mtl);

//    //  Free arrays
//    free(V);
//    free(T);
//    free(N);

   return 0;
}