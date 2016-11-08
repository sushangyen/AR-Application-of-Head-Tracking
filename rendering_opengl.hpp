#include <math.h>

class Rendering{
public:
	int myrender(){
		// Initialise GLFW
		if( !glfwInit() )
		{
			fprintf( stderr, "Failed to initialize GLFW\n" );
			return -1;
		}

		glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
		glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
		glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
		glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		// Open a window and create its OpenGL context
		if( !glfwOpenWindow( 640, 480, 0,0,0,0, 32,0, GLFW_WINDOW ) )
		//if( !glfwOpenWindow( 1920, 1080, 0,0,0,0, 32,0, GLFW_FULLSCREEN  ) )
		{
			fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
			glfwTerminate();
			return -1;
		}

		// Initialize GLEW
		glewExperimental = true; // Needed for core profile  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		if (glewInit() != GLEW_OK) {
			fprintf(stderr, "Failed to initialize GLEW\n");
			return -1;
		}

		glfwSetWindowTitle( "Suzanne" );

		// Ensure we can capture the escape key being pressed below
		glfwEnable( GLFW_STICKY_KEYS );
		//glfwSetMousePos(800/2, 800/2);

		// Dark blue background
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

		// Enable depth test
		glEnable(GL_DEPTH_TEST);
		// Accept fragment if it closer to the camera than the former one
		glDepthFunc(GL_LESS); 

		// Cull triangles which normal is not towards the camera
		glEnable(GL_CULL_FACE);

		GLuint VertexArrayID;
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		// Create and compile our GLSL program from the shaders
		GLuint programID = LoadShaders( "StandardShading.vertexshader", "StandardShadingChanged.fragmentshader" );

		// Get a handle for our "MVP" uniform
		GLuint MatrixID = glGetUniformLocation(programID, "MVP");
		GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
		GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

		// Load the texture
		//GLuint Texture = loadDDS("uvmap.DDS"); // AR
		GLuint Texture = loadBMP_custom("mask.bmp"); // AR

		// Get a handle for our "myTextureSampler" uniform
		GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

		map<string,Object> objmap;
		set<string> objname;
		string cd;
		map<string,Material> matname;
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec2> uvs;
		std::vector<glm::vec3> normals;
		//std::vector<glm::vec3> facesMask;
		ReadObj2(cd,".\\M\\mask.obj",objmap,objname,matname, vertices, uvs, normals);

		//scale, translation, rotation
		float scale = 5.7;
		float tx = 0;
		float ty = -85;
		float tz = 125;
		float ax = 0;
		float ay = 0;
		float az = 0;
		int temp_x, temp_y, temp_z;

		for(int i = 0; i<vertices.size(); i++)          //rotation
		{
			temp_x = vertices[i].x;
			temp_y = vertices[i].y;
			temp_z = vertices[i].z;
			vertices[i].x = cos(ay)*cos(az)*temp_x + (sin(ax)*sin(ay)*cos(az)-cos(ax)*sin(az))*temp_y + (cos(ax)*sin(ay)*cos(az)+sin(ax)*sin(az))*temp_z;
			vertices[i].y = cos(ay)*sin(az)*temp_x + (sin(ax)*sin(ay)*sin(az)+cos(ax)*cos(az))*temp_y + (cos(ax)*sin(ay)*sin(az)-sin(ax)*cos(az))*temp_z;
			vertices[i].z = -sin(ay)*temp_x + sin(ax)*cos(ay)*temp_y + cos(ax)*cos(ay)*temp_z;
		}

		for(int i=0; i<vertices.size(); i++)		//translation & scale
		{
			vertices[i].x = scale*vertices[i].x + tx;
			vertices[i].y = scale*vertices[i].y + ty;
			vertices[i].z = scale*vertices[i].z + tz;
		}


		// Load it into a VBO
		GLuint vertexbuffer;
		glGenBuffers(1, &vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

		GLuint uvbuffer;
		glGenBuffers(1, &uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

		GLuint normalbuffer;
		glGenBuffers(1, &normalbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

		// Get a handle for our "LightPosition" uniform
		glUseProgram(programID);
		GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");


		//IplImage *iplImagedump;
		iplImagedump = cvCreateImage(cvSize(640,480), IPL_DEPTH_8U, 3);
		//cv::Mat imgcolor(cv::Size(640,480),CV_8UC3);
		do{

			// Clear the screen
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Use our shader
			glUseProgram(programID);

			
			glm::mat4 ModelMatrix = glm::mat4(1.0);
			
			//cv::Mat imgInput;
			//imgInput = imgOPENGL.clone();
			glm::mat4 ViewMatrix;
			{
						boost::mutex::scoped_lock lk(mutexViewPoint); // lock
						glm::vec3 lookatpoint(0,0,1);// replace with the screen-camera calibration result
						ModelMatrix = ModelFaceGlobOPENGL;
						
						//trans_camera_coordinate(ModelMatrix,lookatpoint);
						ViewMatrix = glm::lookAt(
							glm::vec3(0,0,0), // Camera is at (4,3,-3), in World Space
							lookatpoint, // and looks at the origin  
							glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
							);
						//printf("%f,%f,%f\n",ModelMatrix[3][0],ModelMatrix[3][1],ModelMatrix[3][2]);
			}


			double focal_length=520.0;
			const double cx=(640-1)*0.5;
			const double cy=(480-1)*0.5;

			glm::mat4 ProjectionMatrix;
			double n,f,t,b,l,r;
			n = 30;
			f = 10000;
			t = n*cy/focal_length;
			b = -n*(480-cy)/focal_length;
			l = -n*cx/focal_length;
			r = n*(640-cx)/focal_length;
			ProjectionMatrix[0][0] = 2*n/(r-l);
			ProjectionMatrix[2][0] = (r+l)/(r-l);
			ProjectionMatrix[1][1] = 2*n/(t-b);
			ProjectionMatrix[2][1] = (t+b)/(t-b);
			ProjectionMatrix[2][2] = -(f+n)/(f-n);
			ProjectionMatrix[3][2] = -2*f*n/(f-n);
			ProjectionMatrix[2][3] = -1;
			ProjectionMatrix[3][3] = 0;

			
			glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
			
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
			glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

			//glm::vec3 lightPos = glm::vec3(4,4,4);
			glm::vec3 lightPos = glm::vec3(0,100,300);
			glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

			// Bind our texture in Texture Unit 0
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, Texture);		//combination
			// Set our "myTextureSampler" sampler to user Texture Unit 0
			glUniform1i(TextureID, 0);

			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);	//combination
			glVertexAttribPointer(
				0,                  // attribute
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
				);

			// 2nd attribute buffer : UVs
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);	//combination
			glVertexAttribPointer(
				1,                                // attribute
				2,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
				);

			// 3rd attribute buffer : normals
			glEnableVertexAttribArray(2);
			glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);	//combination 
			glVertexAttribPointer(
				2,                                // attribute
				3,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
				);

			// Draw the triangles !
			glDrawArrays(GL_TRIANGLES, 0, vertices.size() );

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);

			// Swap buffers
			glfwSwapBuffers();

			cv::Mat depth32FC1(cv::Size(640,480),CV_32FC1);
			WindowZbufferDump2Mem(depth32FC1);
			cv::Mat depthWarped16U(cv::Size(640,480),CV_16UC1);
			// Recover the 16U depth
			float zFar = f;
			float zNear = n;
			for (int y=0; y<480; y++)
			{
				for (int x=0; x<640; x++)
				{
					float z_b = depth32FC1.at<float>(y,x);
					float z_e;
					if(z_b!=1)
						z_e = 2*zFar*zNear / (zFar + zNear - (zFar - zNear)*(2*z_b -1));
					else
						z_e = 0; // area outside the far plane is treated as hole

					depthWarped16U.at<unsigned short>(y,x) = (unsigned short)z_e;
				}
			}
			cv::Mat depth8U;
			depthWarped16U.convertTo(depth8U,CV_8U,-255.0/1000.0,255);
			imshow("Rendered depth image", depth8U);
			cv::waitKey(1);

			// kinect depth map
			dep16.convertTo(depth8U,CV_8U,-255.0/1000.0,255);
			imshow("Kinect depth", depth8U);
			cv::waitKey(1);

			//IplImage *iplImagedump;
			//iplImagedump = cvCreateImage(cvSize(640,480), IPL_DEPTH_8U, 3);
			WindowDump2(iplImagedump);
			//cv::imshow("imgcolor",imgcolor);
			char * ptr = iplImagedump->imageData;
			uchar* rgb = imgcolor.data;
			for(int j=0; j<480; j++)
			{
				for(int i=0; i<640; i++)
				{
					if ((dep16.at<unsigned short>(j,i) < depthWarped16U.at<unsigned short>(j,i)) && dep16.at<unsigned short>(j,i) != 0)
					{
						uchar B = rgb[j*640*3+i*3+0];
						uchar G = rgb[j*640*3+i*3+1];
						uchar R = rgb[j*640*3+i*3+2];

						ptr[j*640*3+3*i+0] = B;
						ptr[j*640*3+3*i+1] = G;
						ptr[j*640*3+3*i+2] = R;
					}

					//if(ptr[j*640*3+3*i+0] == 0 && ptr[j*640*3+3*i+1] == 0 && ptr[j*640*3+3*i+2] == 0)
					if(depthWarped16U.at<unsigned short>(j,i) == 0)
					{
						//cv::Vec3b pix = imgcolor.at(j,i);

						uchar B = rgb[j*640*3+i*3+0];
						uchar G = rgb[j*640*3+i*3+1];
						uchar R = rgb[j*640*3+i*3+2];

						ptr[j*640*3+3*i+0] = B;
						ptr[j*640*3+3*i+1] = G;
						ptr[j*640*3+3*i+2] = R;
					}
				}
			}
			cvShowImage("AR",iplImagedump);

		} // Check if the ESC key was pressed or the window was closed
		while( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS &&
			glfwGetWindowParam( GLFW_OPENED ) );

		// Cleanup VBO and shader
		glDeleteBuffers(1, &vertexbuffer);
		glDeleteBuffers(1, &uvbuffer);
		glDeleteBuffers(1, &normalbuffer);
		glDeleteProgram(programID);
		glDeleteTextures(1, &Texture);
		glDeleteVertexArrays(1, &VertexArrayID);

		// Close OpenGL window and terminate GLFW
		glfwTerminate();

		cvReleaseImage(&iplImagedump);

		return 0;

}

	//AR
	int myrender2(){
		// Initialise GLFW
		if( !glfwInit() )
		{
			fprintf( stderr, "Failed to initialize GLFW\n" );
			return -1;
		}

		glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
		glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
		glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
		glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		// Open a window and create its OpenGL context
		if( !glfwOpenWindow( 640, 480, 0,0,0,0, 32,0, GLFW_WINDOW ) )
		//if( !glfwOpenWindow( 1920, 1080, 0,0,0,0, 32,0, GLFW_FULLSCREEN  ) )
		{
			fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
			glfwTerminate();
			return -1;
		}

		// Initialize GLEW
		glewExperimental = true; // Needed for core profile  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		if (glewInit() != GLEW_OK) {
			fprintf(stderr, "Failed to initialize GLEW\n");
			return -1;
		}

		glfwSetWindowTitle( "Suzanne" );

		// Ensure we can capture the escape key being pressed below
		glfwEnable( GLFW_STICKY_KEYS );
		//glfwSetMousePos(800/2, 800/2);

		// Dark blue background
		glClearColor(0.0f, 0.0f, 0.3f, 0.0f);

		// Enable depth test
		glEnable(GL_DEPTH_TEST);
		// Accept fragment if it closer to the camera than the former one
		glDepthFunc(GL_LESS); 

		// Cull triangles which normal is not towards the camera
		glEnable(GL_CULL_FACE);

		GLuint VertexArrayID;
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		// Create and compile our GLSL program from the shaders
		GLuint programID = LoadShaders( "StandardShading.vertexshader", "StandardShadingChanged.fragmentshader" );

		// Get a handle for our "MVP" uniform
		GLuint MatrixID = glGetUniformLocation(programID, "MVP");
		GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
		GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

		// Load the texture
		GLuint Texture = loadDDS("uvmap.DDS");

		// Get a handle for our "myTextureSampler" uniform
		GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

		// Read our .obj file
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec2> uvs;
		std::vector<glm::vec3> normals;
		//bool res = loadOBJ("suzanne.obj", vertices, uvs, normals);
		bool res = loadOBJ_TRex("Trex_triangle2.obj", vertices, uvs, normals);
		//bool res = loadOBJ("Trex_triangle.obj", vertices, uvs, normals);
		GLfloat g_vertex_buffer_data[] = { 
		-1.0f, 1.0f,  1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f,  1.0f,
		-1.0f, -1.0f, 1.0f,

		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f,-1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f,-1.0f,
		-1.0f, -1.0f, 1.0f,

		1.0f,-1.0f,1.0f,
		-1.0f, -1.0f,-1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f,-1.0f,1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f,-1.0f,

		1.0f,-1.0f,-1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, -1.0f,1.0f,
		-1.0f, -1.0f, -1.0f,

		1.0f,1.0f,-1.0f,
		1.0f, -1.0f,1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,1.0f,-1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,1.0f,

		1.0f,1.0f,-1.0f,
		1.0f, 1.0f,1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f,1.0f,-1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f,1.0f,

		-1.0f,1.0f,-1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f,-1.0f,
		-1.0f,1.0f,-1.0f,
		1.0f, 1.0f,-1.0f,
		1.0f, 1.0f, 1.0f,

		-1.0f,1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f,1.0f,
		-1.0f,1.0f,-1.0f,
		1.0f, 1.0f,1.0f,
		-1.0f, 1.0f, 1.0f,
		};
		GLfloat g_norm_buffer_data[] = { 
		-1.0f, 0.0f,  0.0f,
		1.0f, 0.0f,  0.0f,

		1.0f, 0.0f,  0.0f,
		-1.0f, 0.0f,  0.0f,

		0.0f,1.0f,0.0f,
		0.0f,-1.0f,0.0f,

		0.0f,1.0f,0.0f,
		0.0f,-1.0f,0.0f,

		1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,

		1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,

		0.0f,1.0f,0.0f,
		0.0f,-1.0f,0.0f,

		0.0f,1.0f,0.0f,
		0.0f,-1.0f,0.0f,
		};
		for(int i=0;i<16;i++)
		{
			glm::vec3 tmpVertex;
			tmpVertex.x=g_vertex_buffer_data[9*i+0];
			tmpVertex.y=g_vertex_buffer_data[9*i+1];
			tmpVertex.z=g_vertex_buffer_data[9*i+2];
			vertices.push_back(tmpVertex);
			tmpVertex.x=g_vertex_buffer_data[9*i+3];
			tmpVertex.y=g_vertex_buffer_data[9*i+4];
			tmpVertex.z=g_vertex_buffer_data[9*i+5];
			vertices.push_back(tmpVertex);
			tmpVertex.x=g_vertex_buffer_data[9*i+6];
			tmpVertex.y=g_vertex_buffer_data[9*i+7];
			tmpVertex.z=g_vertex_buffer_data[9*i+8];
			vertices.push_back(tmpVertex);
			
			glm::vec2 tmpUV;
			tmpUV.x=0;
			tmpUV.y=0;
			uvs.push_back(tmpUV);
			uvs.push_back(tmpUV);
			uvs.push_back(tmpUV);
			
			glm::vec3 tmpNorm;
			tmpNorm.x=g_norm_buffer_data[3*i+0];
			tmpNorm.y=g_norm_buffer_data[3*i+1];
			tmpNorm.z=g_norm_buffer_data[3*i+2];
			normals.push_back(tmpNorm);
			normals.push_back(tmpNorm);
			normals.push_back(tmpNorm);
		}
		
		//printf("abcde: %d\n",vertices.size());

		// Load it into a VBO
		GLuint vertexbuffer;
		glGenBuffers(1, &vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

		GLuint uvbuffer;
		glGenBuffers(1, &uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

		GLuint normalbuffer;
		glGenBuffers(1, &normalbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

		// Get a handle for our "LightPosition" uniform
		glUseProgram(programID);
		GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

		do{

			// Clear the screen
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Use our shader
			glUseProgram(programID);

			glm::mat4 ModelMatrix = glm::mat4(1.0);
			GLfloat matrix_data[] = { 30.0895349621437,	0.164973871669801,	-0.480821888792977,	6.79035525588019,
-0.197732523933180,	16.9133424182398,	2.52187090118353,	-28.5328826154107,
0.754569696654994,	-2.14648057313198,	19.8343015868127,	-15.8772936857988,
0,	0,	0,	1
};
			int count = 0;
			for(int i=0;i<4;i++)
			{
				for(int j=0;j<4;j++)
				{
					ModelMatrix[j][i]=matrix_data[count];
					count++;
				}
			}

			glm::mat4 ViewMatrix;
			{
						boost::mutex::scoped_lock lk(mutexViewPoint); // lock
						double x = -ViewPointXGlob/10;
						double y = ViewPointYGlob/10;
						double z = ViewPointZGlob/10;
						//printf("%f,%f,%f\n",x,y,z);
						glm::vec3 lookatpoint(0,0,1);// replace with the screen-camera calibration result
						trans_camera_coordinate(ModelMatrix,lookatpoint);
						ViewMatrix = glm::lookAt(
							glm::vec3(x,y,z), // Camera is at (4,3,-3), in World Space
							lookatpoint, // and looks at the origin  
							glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
							);
			}

			
			glm::mat4 ProjectionMatrix;
			float l;
			float r;
			float b;
			float t;
			float n;
			float f;
            
			n=0.1;
			f=10000;
			glm::vec3 va,vb,vc,vr,vu,vn;
			glm::vec3 pa(-1.0f,-1.0f,1.0f); // replace with the screen-camera calibration result
			glm::vec3 pb(1.0f,-1.0f,1.0f);	// replace with the screen-camera calibration result
			glm::vec3 pc(-1.0f,1.0f,1.0f);	// replace with the screen-camera calibration result
			//ModelMatrix
			glm::vec3 pe(0.0f,0.0f,0.0f);
			trans_camera_coordinate(ViewMatrix*ModelMatrix,pa);
			trans_camera_coordinate(ViewMatrix*ModelMatrix,pb);
			trans_camera_coordinate(ViewMatrix*ModelMatrix,pc);
			//trans_camera_coordinate(ViewMatrix,pe);
			//printf("pe:\n%f,%f,%f\n",pe.x,pe.y,pe.z);
			vr = pb-pa;
			vu = pc-pa;
			vr = glm::normalize(vr);
			vu = glm::normalize(vu);
			vn = glm::cross(vr,vu);
			vn = glm::normalize(vn);
			va = pa-pe;
			vb = pb-pe;
			vc = pc-pe;
			float d = -glm::dot(va,vn);
			l = glm::dot(vr,va)*n/d;
			r = glm::dot(vr,vb)*n/d;
			b = glm::dot(vu,va)*n/d;
			t = glm::dot(vu,vc)*n/d;
			//printf("%f,%f,%f,%f\n",l,r,b,t);
			glm::mat4 M=glm::mat4(1.0f);
			glm::mat4 T=glm::mat4(1.0f);
			M[0][0]=vr[0];M[0][1]=vr[1];M[0][2]=vr[2];
			M[1][0]=vu[0];M[1][1]=vu[1];M[1][2]=vu[2];
			M[2][0]=vn[0];M[2][1]=vn[1];M[2][2]=vn[2];
			T[0][3]=-pe[0];
			T[1][3]=-pe[1];
			T[2][3]=-pe[2];
			OpenGlFrustum(l,  r,  b,  t,  n,  f, ProjectionMatrix);

			ProjectionMatrix = ProjectionMatrix*glm::transpose(M)*glm::transpose(T);
			
			glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
			//glm::mat4 MVP = ProjectionMatrix * ModelMatrix;

			//glFrustum(-100,100,-100,100,0.1,10000);

			// Send our transformation to the currently bound shader, 
			// in the "MVP" uniform
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
			glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

			//glm::vec3 lightPos = glm::vec3(4,4,4);
			glm::vec3 lightPos = glm::vec3(10,-15,15);
			glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

			// Bind our texture in Texture Unit 0
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, Texture);
			// Set our "myTextureSampler" sampler to user Texture Unit 0
			glUniform1i(TextureID, 0);

			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
			glVertexAttribPointer(
				0,                  // attribute
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
				);

			// 2nd attribute buffer : UVs
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
			glVertexAttribPointer(
				1,                                // attribute
				2,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
				);

			// 3rd attribute buffer : normals
			glEnableVertexAttribArray(2);
			glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
			glVertexAttribPointer(
				2,                                // attribute
				3,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
				);

			// Draw the triangles !
			glDrawArrays(GL_TRIANGLES, 0, vertices.size() );

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);

			// Swap buffers
			glfwSwapBuffers();

		} // Check if the ESC key was pressed or the window was closed
		while( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS &&
			glfwGetWindowParam( GLFW_OPENED ) );

		// Cleanup VBO and shader
		glDeleteBuffers(1, &vertexbuffer);
		glDeleteBuffers(1, &uvbuffer);
		glDeleteBuffers(1, &normalbuffer);
		glDeleteProgram(programID);
		glDeleteTextures(1, &Texture);
		glDeleteVertexArrays(1, &VertexArrayID);

		// Close OpenGL window and terminate GLFW
		glfwTerminate();

		return 0;
	}


	Rendering(){
		cv::Mat imgcolortmp(cv::Size(640,480),CV_8UC3);
		cv::Mat dep16tmp(cv::Size(640,480),CV_16U);
		imgcolor = imgcolortmp.clone();
		dep16 = dep16tmp.clone();
	}
	~Rendering(){
	}

	void set_imgcolor(cv::Mat img)
	{
		//imgcolor = img.clone();
		memcpy(imgcolor.data, img.data, 640*480*3);
	}

	void set_dep16(cv::Mat depth16U)
	{
		//imgcolor = img.clone();
		memcpy(dep16.data, depth16U.data, 640*480*2);
	}

	//AR
	void set_mean_face_temple(std::vector<glm::vec3> vectors, std::vector<glm::vec3> normals, std::vector<glm::vec3> faces)
	{
		for (int i=0; i<faces.size(); i++)
		{
			int vidx1 = faces[i].x - 1;
			int vidx2 = faces[i].y - 1;
			int vidx3 = faces[i].z - 1;

			glm::vec2 tmpvec2;
			tmpvec2.x = 0.5;
			tmpvec2.y = 0.5;

			vectorsBFM.push_back(vectors[vidx1]);
			normalsBFM.push_back(normals[vidx1]);
			uvsBFM.push_back(tmpvec2);

			vectorsBFM.push_back(vectors[vidx2]);
			normalsBFM.push_back(normals[vidx2]);
			uvsBFM.push_back(tmpvec2);

			vectorsBFM.push_back(vectors[vidx3]);
			normalsBFM.push_back(normals[vidx3]);
			uvsBFM.push_back(tmpvec2);
		}
	}

private:

	std::vector<glm::vec3> vectorsBFM;	// for opengl display  //AR
	std::vector<glm::vec3> normalsBFM;  //AR
	std::vector<glm::vec2> uvsBFM;		//AR

	IplImage *iplImagedump;
	cv::Mat imgcolor;
	cv::Mat dep16;

	void OpenGlFrustum(float l, float r, float b, float t, float n, float f, glm::mat4 &mat)
	{
		mat[0][0] = 2 * n / (r - l);
		mat[0][1] = 0;
		mat[0][2] = 0;
		mat[0][3] = 0;

		mat[1][0] = 0;
		mat[1][1] = 2 * n / (t - b);
		mat[1][2] = 0;
		mat[1][3] = 0;

		mat[2][0] = (r + l) / (r - l);
		mat[2][1] = (t + b) / (t - b);
		mat[2][2] = -(f + n) / (f - n);
		mat[2][3] = -1;

		mat[3][0] = 0;
		mat[3][1] = 0;
		mat[3][2] = -2 * f * n / (f - n);
		mat[3][3] = 0;
	}
	
	void trans_camera_coordinate(glm::mat4 ViewMatrix, glm::vec3 &p)
	{
		glm::vec4 tmp;
		tmp.x = p.x;
		tmp.y = p.y;
		tmp.z = p.z;
		tmp.w = 1;

		tmp = ViewMatrix*tmp;
		p.x = tmp.x/tmp.w;
		p.y = tmp.y/tmp.w;
		p.z = tmp.z/tmp.w;
	}

	int WindowDump2(IplImage *iplImageShow)
{
   int i,j;
   FILE *fptr;
   static int counter = 0; /* This supports animation sequences */
   char fname[32];
   unsigned char *image;

   int width=640;
   int height=480;
   int stereo=0;

   /* Allocate our buffer for the image */
   if ((image = (unsigned char *)malloc(3*width*height*sizeof(char))) == NULL) {
      fprintf(stderr,"Failed to allocate memory for image\n");
      return(FALSE);
   }

   glPixelStorei(GL_PACK_ALIGNMENT,1);

   /* Copy the image into our buffer */
   glReadBuffer(GL_BACK_LEFT);
   glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,image);

   /* Write the raw file */
   /* fprintf(fptr,"P6\n%d %d\n255\n",width,height); for ppm */
   //IplImage *iplImageShow;
   //iplImageShow = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, 3);
   //memcpy(iplImageShow->imageData, image, width*height*3);
   char* ptr=iplImageShow->imageData;
   for (j=height-1;j>=0;j--) {
      for (i=0;i<width;i++) {
         //fputc(image[3*j*width+3*i+0],fptr);
         //fputc(image[3*j*width+3*i+1],fptr);
         //fputc(image[3*j*width+3*i+2],fptr);
		 *ptr++=image[3*j*width+3*i+2];
		 *ptr++=image[3*j*width+3*i+1];
		 *ptr++=image[3*j*width+3*i+0];
      }
   }
  

   /* Clean up */
   counter++;
   free(image);
   return(TRUE);
}
    
	int WindowZbufferDump2Mem(cv::Mat img)
	{
		//IplImage *iplImageShow;
		int width;
		int height;

		width = img.cols;
		height = img.rows;

		int i,j;
		FILE *fptr;
		static int counter = 0; /* This supports animation sequences */
		char fname[32];
		//unsigned char *image;
		float *image;

		int stereo=0;

		/* Allocate our buffer for the image */
		//if ((image = (unsigned char *)malloc(3*width*height*sizeof(char))) == NULL) {
		//	fprintf(stderr,"Failed to allocate memory for image\n");
		//	return(FALSE);
		//}
		//if ((image = (float *)malloc(3*width*height*sizeof(float))) == NULL) {
		if ((image = (float *)malloc(width*height*sizeof(float))) == NULL) {
			fprintf(stderr,"Failed to allocate memory for image\n");
			return(FALSE);
		}

		glPixelStorei(GL_PACK_ALIGNMENT,1);

		/* Copy the image into our buffer */
		glReadPixels(0,0,width,height,GL_DEPTH_COMPONENT,GL_FLOAT,image);
		/* Write the raw file */
		/* fprintf(fptr,"P6\n%d %d\n255\n",width,height); for ppm */
		//char* ptr=iplImageShow->imageData;
		float* ptr = (float*)img.data;
		for (j=height-1;j>=0;j--) {
		//for (j=0;j<height;j++) {
			for (i=0;i<width;i++) {
				*ptr++=image[j*width+i];

			}
		}

		/* Clean up */
		counter++;
		free(image);
		return(TRUE);
	}
};

