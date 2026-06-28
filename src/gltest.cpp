// #ifndef GL_TEST
// #define GL_TEST

// #include <glad/glad.h>
// #include <GLFW/glfw3.h>
// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtc/type_ptr.hpp>
// #include <iostream>
// #include<math.h>

// #include <fstream>
// #include <sstream>
// #include <string>

// float fov = 45;

// void printMat4(const glm::mat4& m)
// {
//     for (int row = 0; row < 4; row++)
//     {
//         for (int col = 0; col < 4; col++)
//         {
//             std::cout << m[col][row] << " ";
//         }
//         std::cout << std::endl;
//     }
//     std::cout << "---------" << std::endl;
// }

// std::string readFile(const char* path)
// {
//     std::ifstream file(path);
//     std::stringstream buffer;
//     buffer << file.rdbuf();
//     return buffer.str();
// }

// // Vertex shader
// const char* vertexShaderSource = R"(
// #version 330 core
// layout (location = 0) in vec3 aPos;
// void main()
// {
//     gl_Position = vec4(aPos, 1.0);
// }
// )";

// // Fragment shader
// const char* fragmentShaderSource = R"(
// #version 330 core
// out vec4 FragColor;
// uniform vec3 color;
// void main()
// {
//     FragColor = vec4(color, 1.0);
// }
// )";

// // const char* shader_path = R"()"

// glm::mat4 projection;
// glm::mat4 view;
// glm::vec3 up_vector = glm::vec3(0,1,0);
// glm::vec3 right_vector = glm::vec3(1,0,0);
// glm::vec3 forward_vector = glm::vec3(0,0,1);

// glm::mat4 model = glm::mat4(1.0f);

// void frameBufferSizeCallBack(GLFWwindow* window, int width, int height) {
//     std::cout<<"window size is " << width << " " << height<<std::endl;
//     projection = glm::perspective(glm::radians(45.0f),(float)width / height, 0.1f,100.0f);
//     glViewport(0,0,width,height);
// }

// void checkExitStatus(GLFWwindow *window, int key) {
//     if(glfwGetKey(window,key) == GLFW_PRESS) {
//         glfwSetWindowShouldClose(window,true);
//     }
// }

// void transformBody(GLFWwindow *window,glm::vec3 *offset,glm::mat4 *view ) {
//     float posx,poy,posz;
//     if(glfwGetKey(window,GLFW_KEY_UP) == GLFW_PRESS) {
//         offset->y = offset->y + 0.02;
//         model = glm::translate(model, glm::vec3(0.0f, 0.02f, 0.0f));
//     } else if(glfwGetKey(window,GLFW_KEY_DOWN) == GLFW_PRESS) {
//         offset->y = offset->y - 0.02;
//         model = glm::translate(model, glm::vec3(0.0f, -0.02f, 0.0f));
//     } else if(glfwGetKey(window,GLFW_KEY_RIGHT) == GLFW_PRESS) {
//         offset->x = offset->x + 0.02;
//         model = glm::translate(model, glm::vec3(0.02f, 0.0f, 0.0f));
//     } else if(glfwGetKey(window,GLFW_KEY_LEFT) == GLFW_PRESS) {
//         offset->x = offset->x - 0.02;
//         model = glm::translate(model, glm::vec3(-0.02f, 0.0f, 0.0f));
//     } else if(glfwGetKey(window,GLFW_KEY_W) == GLFW_PRESS) {
//         *view = glm::translate(*view, glm::vec3(0.0f, 0.2f, 0.0f));
//     } else if(glfwGetKey(window,GLFW_KEY_S) == GLFW_PRESS) {
//         *view = glm::translate(*view, glm::vec3(0.0f, -0.2f, 0.0f));
//     } else if(glfwGetKey(window,GLFW_KEY_A) == GLFW_PRESS) {
//         *view = glm::translate(*view, glm::vec3(-0.2f, 0.0f, 0.0f));
//     } else if(glfwGetKey(window,GLFW_KEY_D) == GLFW_PRESS) {
//         *view = glm::translate(*view, glm::vec3(0.2f, 0.0f, 0.0f));
//     } else if(glfwGetKey(window,GLFW_KEY_R) == GLFW_PRESS) {
//         *view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f));
//         up_vector = glm::vec3(0,1,0);
//         right_vector = glm::vec3(1,0,0);
//         forward_vector = glm::vec3(0,0,1);
//     } 
// }

// void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
// {
//     fov -= (float)yoffset;

//     if (fov < 1.0f)  fov = 1.0f;
//     if (fov > 90.0f) fov = 90.0f;

//     projection = glm::perspective(glm::radians(fov),(float)800 / 600, 0.1f,100.0f);
// }

// bool isDragging = false;
// bool firstMouse = true;

// double lastX = 0, lastY = 0;

// void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
// {
//     if (button == GLFW_MOUSE_BUTTON_LEFT)
//     {
//         if (action == GLFW_PRESS)
//         {
//             isDragging = true;
//             firstMouse = true; // RESET HERE
//         }
//         else if (action == GLFW_RELEASE)
//         {
//             isDragging = false;
//         }
//     }
// }

// void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
// {
//     if (!isDragging) return; // ignore movement if not dragging

//     if (firstMouse)
//     {
//         lastX = xpos;
//         lastY = ypos;
//         firstMouse = false;
//         return; // avoid jump
//     }

//     double dx = xpos - lastX;
//     double dy = ypos - lastY;
//     glm::mat4 R;

//     view = glm::rotate(view, (float)(dy * 0.01), right_vector);
//     R = glm::rotate(glm::mat4(1.0f), (float)(dy * 0.01), right_vector);
//     up_vector = glm::vec3(R * glm::vec4(up_vector, 1.0f));
//     view = glm::rotate(view, (float)(dx * 0.01), up_vector);
//     R = glm::rotate(glm::mat4(1.0f), (float)(dx * 0.01), up_vector);
//     right_vector = glm::vec3(R * glm::vec4(right_vector, 1.0f));

//     lastX = xpos;
//     lastY = ypos;

//     // use dx, dy
// }
// int testgl()
// {
//     // Initialize GLFW
//     if (!glfwInit()) {
//         std::cout << "Failed to init GLFW\n";
//         return -1;
//     }

//     // Set OpenGL version (IMPORTANT)
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//     glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

//     // Create window
//     GLFWwindow* window = glfwCreateWindow(800, 600, "Triangle", NULL, NULL);
//     if (!window) {
//         std::cout << "Failed to create window\n";
//         glfwTerminate();
//         return -1;
//     }

//     glfwMakeContextCurrent(window);
//     glfwSetFramebufferSizeCallback(window,frameBufferSizeCallBack);

//     // Initialize GLAD
//     if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
//         std::cout << "Failed to init GLAD\n";
//         return -1;
//     }

//     // Print OpenGL version
//     std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

//     // Triangle vertices
//     float vertices[] = {
//     // back face
//     -0.5f, -0.5f, -0.5f,
//      0.5f, -0.5f, -0.5f,
//      0.5f,  0.5f, -0.5f,
//      0.5f,  0.5f, -0.5f,
//     -0.5f,  0.5f, -0.5f,
//     -0.5f, -0.5f, -0.5f,

//     // front face
//     -0.5f, -0.5f,  0.5f,
//      0.5f, -0.5f,  0.5f,
//      0.5f,  0.5f,  0.5f,
//      0.5f,  0.5f,  0.5f,
//     -0.5f,  0.5f,  0.5f,
//     -0.5f, -0.5f,  0.5f,

//     // left face
//     -0.5f,  0.5f,  0.5f,
//     -0.5f,  0.5f, -0.5f,
//     -0.5f, -0.5f, -0.5f,
//     -0.5f, -0.5f, -0.5f,
//     -0.5f, -0.5f,  0.5f,
//     -0.5f,  0.5f,  0.5f,

//     // right face
//      0.5f,  0.5f,  0.5f,
//      0.5f,  0.5f, -0.5f,
//      0.5f, -0.5f, -0.5f,
//      0.5f, -0.5f, -0.5f,
//      0.5f, -0.5f,  0.5f,
//      0.5f,  0.5f,  0.5f,

//     // bottom face
//     // -0.5f, -0.5f, -0.5f,
//     //  0.5f, -0.5f, -0.5f,
//     //  0.5f, -0.5f,  0.5f,
//     //  0.5f, -0.5f,  0.5f,
//     // -0.5f, -0.5f,  0.5f,
//     // -0.5f, -0.5f, -0.5f,

//     // top face
//     // -0.5f,  0.5f, -0.5f,
//     //  0.5f,  0.5f, -0.5f,
//     //  0.5f,  0.5f,  0.5f,
//     //  0.5f,  0.5f,  0.5f,glm::rotate(model, angle, glm::vec3(0,0,1));
//     // -0.5f,  0.5f,  0.5f,
//     // -0.5f,  0.5f, -0.5f
// };

//     float vertices_line[] = {
//         //x axis
//         0.0,0.0,0.0,
//         1.0,0.0,0.0,
//         //y axis
//         0.0,0.0,0.0,
//         0.0,1.0,0.0,
//         //z axis
//         0.0,0.0,0.0,
//         0.0,0.0,1.0,


//     };

//     float dt = 0.1;
//     float time = 0;
//     float v = 0.5;
//     float w = 0.3;
//     float theta = 0;
//     float qx = 0,qy = 0,qz = 0,qw = 0;
//     float posx = 0,posy = 0,posz = 0;

    
//     glm::mat4 model_axes = glm::mat4(1.0f); // identity (no rotation)
//     // model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));

//     view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f));

//     projection = glm::perspective(glm::radians(45.0f),(float)800 / 600, 0.1f,100.0f);

//     glm::vec3 tr_offset = glm::vec3(0,0,0);

//     std::string vertexCode = readFile(R"(/home/srinidhi/Documents/Simulations/Testing/Project1/src/shaders/vertexShader.glsl)");

//     // Create buffers
//     unsigned int VAO, VBO,VAO2,VBO2;
//     glGenVertexArrays(1, &VAO);
//     glGenBuffers(1, &VBO);

//     glGenVertexArrays(1, &VAO2);
//     glGenBuffers(1, &VBO2);

//     // Bind VAO first
//     glBindVertexArray(VAO);

//     // Bind and fill VBO
//     glBindBuffer(GL_ARRAY_BUFFER, VBO);
//     glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

//     // Describe vertex layout
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
//     glEnableVertexAttribArray(0);

//     glBindVertexArray(VAO2);
//     glBindBuffer(GL_ARRAY_BUFFER, VBO2);
//     glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_line), vertices_line, GL_STATIC_DRAW);

//     // Describe vertex layout
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
//     glEnableVertexAttribArray(0);

    

//     // Compile vertex shader
//     unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
//     const char* vertexShaderSource = vertexCode.c_str();
//     glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
//     glCompileShader(vertexShader);

//     // Compile fragment shader
//     unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//     glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
//     glCompileShader(fragmentShader);

//     // Link shaders
//     unsigned int shaderProgram = glCreateProgram();
//     glAttachShader(shaderProgram, vertexShader);
//     glAttachShader(shaderProgram, fragmentShader);
//     glLinkProgram(shaderProgram);

//     // Cleanup shaders (no longer needed after linking)
//     glDeleteShader(vertexShader);
//     glDeleteShader(fragmentShader);

//     int quatLoc = glGetUniformLocation(shaderProgram, "quat");
//     int offset = glGetUniformLocation(shaderProgram, "offset");
//     int colorLoc = glGetUniformLocation(shaderProgram, "color");

//     int modelLoc = glGetUniformLocation(shaderProgram, "model");
//     int viewLoc  = glGetUniformLocation(shaderProgram, "view");
//     int projLoc  = glGetUniformLocation(shaderProgram, "projection");

//     glEnable(GL_DEPTH_TEST);
//     glDisable(GL_CULL_FACE);

//     glfwSetScrollCallback(window, scroll_callback);
//     glfwSetMouseButtonCallback(window, mouse_button_callback);
//     glfwSetCursorPosCallback(window, cursor_position_callback);

    

//     // Render loop
//     while (!glfwWindowShouldClose(window))
//     {
//         checkExitStatus(window,GLFW_KEY_ESCAPE);
//         transformBody(window,&tr_offset,&view);

//         glClearColor(0,0,0,1);
//         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//         // glUniform4f(quatLoc, qx, qy, qz, qw);
//         // glUniform3f(offset,posz,posy,posz);
        

//         // glUseProgram(shaderProgram);
//         // glBindVertexArray(VAO);

//         // glDrawArrays(GL_TRIANGLES, 0, 36);
//         // glBindVertexArray(VAO2);

//         // glDrawArrays(GL_LINE_STRIP, 0, 5);

//         glUseProgram(shaderProgram);
//         glBindVertexArray(VAO);
//         // glUniform3f(offset,tr_offset.x,tr_offset.y,tr_offset.z);

//         glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
//         glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
//         glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        

//         // 1. Draw filled cube
//         glUniform3f(colorLoc,0.5,0.25,0.0);
//         glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//         glDrawArrays(GL_TRIANGLES, 0, 36);

//         // 2. Draw edges on top
//         glUniform3f(colorLoc,0.5,0.5,0.5);
//         glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//         glLineWidth(2.0f);

//         // Optional: make edges always visible
//         glDisable(GL_DEPTH_TEST);

//         glDrawArrays(GL_TRIANGLES, 0, 36);

//         glEnable(GL_DEPTH_TEST);
//         glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

//         glBindVertexArray(VAO2);
//         glUniform3f(offset,0,0,0);
//         glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model_axes));
//         glUniform3f(colorLoc,1,0.25,0.0);
//         glLineWidth(2.0f);
//         glDisable(GL_DEPTH_TEST);
//         glDrawArrays(GL_LINES,0,6);
//         glEnable(GL_DEPTH_TEST);
//         time = time + dt;
//         theta = w*dt;

//         qw = cos(theta/2);
//         qz = sin(theta/2);

//         // posy = posy + v*dt;
//         // if(posy > 10) {
//         //     posy = -10;
//         // }
//         // model = glm::rotate(model, theta, glm::vec3(0.0f, 0.0f, 1.0f));
//         float angle = glfwGetTime();

//         // model = glm::translate(model, tr_offset);

//         // model = glm::mat4(1.0f);
//         // model = glm::rotate(model, angle, glm::vec3(0,0,1));
//         // model = glm::translate(model,glm::vec3(posx,posy,posz));


//         glfwSwapBuffers(window);
//         glfwPollEvents();
//     }

//     // Cleanup
//     glDeleteVertexArrays(1, &VAO);
//     glDeleteBuffers(1, &VBO);

//     glfwTerminate();
//     return 0;
// }

// #endif