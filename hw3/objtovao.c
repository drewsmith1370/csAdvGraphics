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

//
//  Read coordinates
//    from : string to read from
//    to   : points to address to write to
//    n    : how many coordinates to read
//
static void readCoord(char* from, GLenum to, GLsizeiptr offset, int n) {
    float temp[3];
    for (int i=0;i<n;i++) {
        char* str = getword(&from);
        if (!str)  Fatal("Premature EOL reading %d floats\n",n);
        if (sscanf(str,"%f",temp+i)!=1) Fatal("Error reading float %d\nString: %s\n",i,str);
    }
    glBufferSubData(to,offset,n,temp);
}

//
// Helper function to create a VAO with static draw for storing an object, using vdata for vbo and idata for ibo
//
GLuint CreateStaticVertexBuffer(int vsize, void* vdata, int isize, void* idata) {
    // Combine as buffer array object
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Make vertex buffer object
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vsize, vdata, GL_STATIC_DRAW);

    // Make Index buffer object
    GLuint ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, isize, idata, GL_STATIC_DRAW);

    // Return name of vao
    return vao;
}

int ObjToVao(char* file, int shader) {
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

    // Generate buffers
    // Get VAO name
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Make vertex buffer object
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex_t[Nv]), NULL, GL_STATIC_DRAW);

    // Make Index buffer object
    GLuint ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int[Ni]), NULL, GL_STATIC_DRAW);
    

    // Reserve space for vbo and ibo
    int Indices[1024];
    // if (!Indices) 
    //     Fatal("Failes to malloc buffers");

    //  Read vertexes and facets
    int v_idx = 0;
    int n_idx = 0;
    // int t_idx = 0;
    int i_idx = 0;
    int i_offset = 0;
    char* str = NULL;
    rewind(f);
    // Read each line and write to corresponding buffers
    while ((line = readline(f)))
    {
        //  Vertex coordinates (always 3)
        if (line[0]=='v' && line[1]==' ')
        {
            readCoord(&line[2],GL_ARRAY_BUFFER,sizeof(Vertex_t)*v_idx+offsetof(Vertex_t,v),3);
            v_idx += 3;
        }
        //  Normal coordinates (always 3)
        else if (line[0]=='v' && line[1] == 'n')
        {
            readCoord(&line[2],GL_ARRAY_BUFFER,sizeof(Vertex_t)*n_idx+offsetof(Vertex_t,n),3);
            n_idx += 3;
        }
        //  Texture coordinates (always 2)
        else if (line[0]=='v' && line[1] == 't');
            // readCoord(&line[2],GL_ARRAY_BUFFER,sizeof(Vertex_t)*t_idx+offsetof(Vertex_t,t),2);
        //  Read and draw facets
        else if (line[0]=='f')
        {
            line++;
            //  Read Vertex/Texture/Normal triplets
            while ((str = getword(&line)))
            {
                int Kv,Kt,Kn;
                //  Try Vertex/Texture/Normal triplet
                if (sscanf(str,"%d/%d/%d",&Kv,&Kt,&Kn)==3)
                {
                    if (Kv != Kt || Kv != Kn) Fatal("Vertex %d/%d/%d: mixed data is unsupported\n",Kv,Kt,Kn);
                }
                //  Try Vertex//Normal pairs
                else if (sscanf(str,"%d//%d",&Kv,&Kn)==2)
                {
                    if (Kv != Kn) Fatal("Vertex %d//%d: mixed data is unsupported\n",Kv,Kn);
                }
                //  Try Vertex index
                else if (sscanf(str,"%d",&Kv)==1)
                {
                    // if (Kv<0 || Kv>Nv/3) Fatal("Vertex %d out of range 1-%d\n",Kv,Nv/3);
                }
                //  This is an error
                else
                    Fatal("Invalid facet %s\n",str);
                //  Draw vectors
                Indices[i_idx] = Kv;
                i_idx++;
            }
            if (i_idx >= 1024-3) {
                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,i_offset,10,Indices);
                i_offset += i_idx;
                i_idx = 0;
            }
        }
        //  Skip this line
    }
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,i_offset,i_idx,Indices);
    i_offset += i_idx;
    i_idx = 0;

    // Free the line buffer
    free(line);
    line=NULL;
    linelen=0;

    // Close the file safely
    fclose(f);
    ErrCheck("obj to vao");

    // Vertex Attributes
    glUseProgram(shader);
    // Get locations of attributes in shader
    int posLoc = glGetAttribLocation(shader,"Pos");
    // int texLoc = glGetAttribLocation(shader,"Tex");
    int nrmLoc = glGetAttribLocation(shader,"Nrm");
    // Enable VAOs
    glEnableVertexAttribArray(posLoc);
    // glEnableVertexAttribArray(texLoc);
    glEnableVertexAttribArray(nrmLoc);
    // Set vertex attribute pointers
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_t), (void*)offsetof(Vertex_t,v));
    // glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_t), (void*)offsetof(Vertex_t,t));
    glVertexAttribPointer(nrmLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_t), (void*)offsetof(Vertex_t,n));
    ErrCheck("attrib ptrs");

    //  Free malloc'd array
    // free(Indices);

   return vao;
}