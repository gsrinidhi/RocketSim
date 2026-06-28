#include "phySim.h"

// Fragment shader
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec3 color;
void main()
{
    FragColor = vec4(color, 1.0);
}
)";

void timer_handler(int, siginfo_t*, void*)
{
    std::cout << "Timer interrupt\n";
}

phySim::phySim() {

}

void phySim::setGuidObj(CGuidance *g) {
    this->g1 = g;
}

double phySim::getPolarAngle() {
    return polar_angle;
}

double phySim::getAzimuthAngle() {
    return azimuth_angle;
}

phyVector phySim::getS() {
    return s;
}

phyVector phySim::getV() {
    return v;
}

double phySim::getFuelMass() {
    return Mf;
}

double phySim::getTimeStep() {
    return dt;
}

void phySim::setLogFile(std::ofstream *logfile) {
    this->logfile = logfile;
}

double phySim::getParam(std::string pname) {
    return configMap[pname];
}

int phySim::simInitFile(std::string fname) {
    std::ifstream file(fname);
    
    // Check if the file opened successfully
    if (!file.is_open()) {
        std::cerr << "Error: Could not open the file." << std::endl;
        return 1;
    }

    std::string line;

    // 2. Read the file line by line
    while (std::getline(file, line)) {
        // Skip empty lines or comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::stringstream ss(line);
        std::string key;
        double value;

        // 3. Extract key and value separated by whitespace
        if (ss >> key >> value) {
            configMap[key] = value;
        }
    }

    // 4. Close the file stream
    file.close();

    dt = configMap["Timestep"];
    Mfo = configMap["Init_Fuel_Mass"];
    Ms = configMap["Init_Str_Mass"];
    Mp = configMap["Payload_Mass"];
    Isp = configMap["Isp"];
    m = configMap["Mass_Flow_Rate"];
    Re = configMap["Earth_Radius"];
    Me = configMap["Earth_Mass"];
    stop_time = configMap["Stop_Time"];
    phi_init = configMap["Init_Polar_Angle"] * PI / 180;
    theta_init = configMap["Init_Inertial_Azimuth"] * PI / 180;
    inclination = configMap["Inclination"] * PI / 180;

    mass = Mfo + Ms + Mp;
    Mf = Mfo;

    g_mag = GRAVITATIONAL_CONSTANT * Me/(Re*Re);
    depletion_flag = 0;

    double sxo = Re*sin(phi_init)*cos(theta_init);
    double syo = Re*sin(phi_init)*sin(theta_init);
    double szo = Re*cos(phi_init);
    s.setXYZ(sxo, syo, szo); // Initial position at Earth's surface
    v.setXYZ(0,0,0);

    return 0;
}


void phySim::simInit(phyVector s, phyVector v) {
    this->s = s;
    this->v = v;
    this->polar_angle = atan2(s.magnitude(1,1,0),s.z);
    this->azimuth_angle = atan2(s.y,s.x);
    time = 0;
}

void phySim::guidInit(std::string gInitFile) {
    g1->setEarthSpecs(Me,Re);
    g1->setAltitudeThresholds(50e3,50e3);
    g1->guidInitFile(gInitFile);
    g1->setLogFile(logfile);
    g1->guidInit(s,phi_init,theta_init,inclination);
}

void phySim::updateLocalFrame(phyVector v,double *local_FPA) {
    polar_angle = atan2(s.magnitude(1,1,0),s.z);
    azimuth_angle = atan2(s.y,s.x);

    Xv.x = -cos(polar_angle) * cos(azimuth_angle);
    Xv.y = -cos(polar_angle) * sin(azimuth_angle);
    Xv.z = sin(polar_angle);

    Yv.x = sin(azimuth_angle);
    Yv.y = -cos(azimuth_angle);

    Zv.x = sin(polar_angle) * cos(azimuth_angle);
    Zv.y = sin(polar_angle) * sin(azimuth_angle);
    Zv.z = cos(polar_angle);

    double vxv = v * Xv;
    double vyv = v * Yv;
    double vzv = v * Zv;

    *local_FPA = atan2(vzv,sqrt(vxv * vxv + vyv * vyv));
}

void phySim::updatePosition(double pitch, double heading,double isThrusting) {

    double r = s.magnitude();
    g_local = GRAVITATIONAL_CONSTANT * Me / (r * r);
    if(isThrusting && depletion_flag == 0) {
        thrustMag = m * Isp * g_mag; // Update thrust magnitude based on current mass flow rate
    } else {
        thrustMag = 0; // No thrust during coasting
    }
    a = Xv * thrustMag/mass * cos(pitch) * cos(heading) + Yv * thrustMag/mass * cos(pitch) * sin(heading) + Zv * (thrustMag/mass * sin(pitch) - g_local);
    v = v + a * dt;
    s.x = s.x + v.x * dt;
    s.y = s.y + v.y * dt;
    s.z = s.z + v.z * dt;
    if(isThrusting && depletion_flag == 0) {
        Mf -= m * dt;
        mass = Ms + Mp + Mf; // Update total mass of the vehicle
        if(Mf < 0 && depletion_flag == 0) {
            Mf = 0; // Ensure fuel mass does not go negative
            mass = Ms + Mp; // Update total mass when fuel is depleted
            depletion_flag = 1; // Stop thrusting when fuel is depleted
            depletion_time = time; // Record the time of fuel depletion for reference
            *logfile << "Depletion time = " << depletion_time << std::endl; 
            *logfile << "Injection velocity = " << v.magnitude() << std::endl;

        }
    }
    time += dt;
}

void frameBufferSizeCallBack(GLFWwindow* window, int width, int height) {
    phySim* renderer =
        static_cast<phySim*>(
            glfwGetWindowUserPointer(window)
        );

    if(!renderer) {
        std::cout<<"Empty pinter\n";
        return;
    }

    renderer->resize(width, height);
}

void phySim::resize(int width, int height)
{
    projection = glm::perspective(
        glm::radians(45.0f),
        (float) width / height,
        1.0f,
        (float)(10.0f * Re_sim)
    );
    glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    phySim* renderer =
        static_cast<phySim*>(
            glfwGetWindowUserPointer(window)
        );

    if(!renderer) {
        std::cout<<"Empty pinter\n";
        return;
    }

    renderer->scroll(xoffset, yoffset);
}

void phySim::scroll(double xoffset,double yoffset) {
    std::cout<<"yoffset = " << yoffset <<std::endl;
    fov -= (float)yoffset;
    std::cout<<"yoffset = " << yoffset <<" fov = " << fov << std::endl;
    if (fov < 1.0f)  fov = 1.0f;
    if (fov > 90.0f) fov = 90.0f;

    projection = glm::perspective(
        glm::radians(fov),
        800.0f/600.0f,
        1.0f,
        (float)(10.0f * Re_sim)
    );
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    phySim* renderer =
        static_cast<phySim*>(
            glfwGetWindowUserPointer(window)
        );

    if(!renderer) {
        std::cout<<"Empty pinter\n";
        return;
    }

    renderer->mouse_button_action(button,action,mods);
}

void phySim::mouse_button_action(int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            isDragging = true;
            firstMouse = true; // RESET HERE
        }
        else if (action == GLFW_RELEASE)
        {
            isDragging = false;
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    phySim* renderer =
        static_cast<phySim*>(
            glfwGetWindowUserPointer(window)
        );

    if(!renderer) {
        std::cout<<"Empty pinter\n";
        return;
    }

    renderer->cursor_position_action(xpos,ypos);
}

void phySim::cursor_position_action(double xpos, double ypos) {
    if (!isDragging) return; // ignore movement if not dragging

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
        return; // avoid jump
    }

    dx = (xpos - lastX) * Re_scale;
    dy = (ypos - lastY) * Re_scale;
    glm::mat4 R;

    // view = glm::rotate(view, (float)(dy * 0.01), right_vector);
    // R = glm::rotate(glm::mat4(1.0f), (float)(dy * 0.01), right_vector);
    // up_vector = glm::vec3(R * glm::vec4(up_vector, 1.0f));
    // view = glm::rotate(view, (float)(dx * 0.01), up_vector);
    // R = glm::rotate(glm::mat4(1.0f), (float)(dx * 0.01), up_vector);
    // right_vector = glm::vec3(R * glm::vec4(right_vector, 1.0f));

    lastX = xpos;
    lastY = ypos;
}

void phySim::checkExitStatus(GLFWwindow* window,int key) {
    if(glfwGetKey(window,key) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window,true);
    }
}

void phySim::transformBody(GLFWwindow *window,glm::vec3 *offset,glm::mat4 *view ) {
    float posx,poy,posz;
    if(glfwGetKey(window,GLFW_KEY_UP) == GLFW_PRESS) {
        offset->y = offset->y + 0.02;
        model = glm::translate(model, glm::vec3(0.0f, 0.02f, 0.0f));
    } else if(glfwGetKey(window,GLFW_KEY_DOWN) == GLFW_PRESS) {
        offset->y = offset->y - 0.02;
        model = glm::translate(model, glm::vec3(0.0f, -0.02f, 0.0f));
    } else if(glfwGetKey(window,GLFW_KEY_RIGHT) == GLFW_PRESS) {
        offset->x = offset->x + 0.02;
        model = glm::translate(model, glm::vec3(0.02f * Re * Re_scale, 0.0f, 0.0f));
    } else if(glfwGetKey(window,GLFW_KEY_LEFT) == GLFW_PRESS) {
        offset->x = offset->x - 0.02;
        model = glm::translate(model, glm::vec3(-0.02f * Re * Re_scale, 0.0f, 0.0f));
    } else if(glfwGetKey(window,GLFW_KEY_W) == GLFW_PRESS) {
        *view = glm::translate(*view, glm::vec3(0.0f, 0.2f * Re * Re_scale, 0.0f));
    } else if(glfwGetKey(window,GLFW_KEY_S) == GLFW_PRESS) {
        *view = glm::translate(*view, glm::vec3(0.0f, -0.2f * Re * Re_scale, 0.0f));
    } else if(glfwGetKey(window,GLFW_KEY_A) == GLFW_PRESS) {
        *view = glm::translate(*view, glm::vec3(-0.2f * Re * Re_scale, 0.0f, 0.0f));
    } else if(glfwGetKey(window,GLFW_KEY_D) == GLFW_PRESS) {
        *view = glm::translate(*view, glm::vec3(0.2f * Re * Re_scale, 0.0f, 0.0f));
    } else if(glfwGetKey(window,GLFW_KEY_R) == GLFW_PRESS) {
        *view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f * Re * Re_scale));
        up_vector = glm::vec3(0,1,0);
        right_vector = glm::vec3(1,0,0);
        forward_vector = glm::vec3(0,0,1);
    } 
}

int phySim::initRender() {
    if (!glfwInit()) {
        std::cout << "Failed to init GLFW\n";
        return -1;
    }

    // Set OpenGL version (IMPORTANT)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    window = glfwCreateWindow(800, 600, "Triangle", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    // glfwSetFramebufferSizeCallback(window,frameBufferSizeCallBack);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to init GLAD\n";
        return -1;
    }

    

    model = glm::mat4(1.0f);

    return 0;
}

void phySim::renderEarth() {
    
}

double phySim::getRadialVelocity() {
    return s * v / s.magnitude();
}

void phySim::populateSphereVertices(double radius,std::vector<float> *vertices,std::vector<unsigned int> *indices) {
    int sectors = 108;
    int stacks = 18;

    for(int i = 0; i <= stacks; i++)
    {
        float stackAngle = M_PI/2 - i * M_PI/stacks;
        float xy = radius * cos(stackAngle);
        float z  = radius * sin(stackAngle);

        for(int j = 0; j <= sectors; j++)
        {
            float sectorAngle = j * 2*M_PI/sectors;

            float x = xy * cos(sectorAngle);
            float y = xy * sin(sectorAngle);

            vertices->push_back(x);
            vertices->push_back(y);
            vertices->push_back(z);
        }
    }

    for(int i = 0; i < stacks; i++)
    {
        int k1 = i * (sectors + 1);
        int k2 = k1 + sectors + 1;

        for(int j = 0; j < sectors; j++, k1++, k2++)
        {
            if(i != 0)
            {
                indices->push_back(k1);
                indices->push_back(k2);
                indices->push_back(k1 + 1);
            }

            if(i != (stacks - 1))
            {
                indices->push_back(k1 + 1);
                indices->push_back(k2);
                indices->push_back(k2 + 1);
            }
        }
    }
}

void phySim::populateCylinderVertices(double radius,double height,std::vector<float> *vertices,std::vector<unsigned int> *indices) {
    int sectors = 36;
    int stacks = 18;

    for(int i = 0; i <= stacks; i++)
    {
        // float stackAngle = M_PI/2 - i * M_PI/stacks;
        // float xy = radius * cos(stackAngle);
        float z  = height * i/stacks;

        for(int j = 0; j <= sectors; j++)
        {
            float sectorAngle = j * 2*M_PI/sectors;

            float x = radius * cos(sectorAngle);
            float y = radius * sin(sectorAngle);

            vertices->push_back(x);
            vertices->push_back(y);
            vertices->push_back(z);
        }
    }

    for(int i = 0; i < stacks; i++)
    {
        int k1 = i * (sectors + 1);
        int k2 = k1 + sectors + 1;

        for(int j = 0; j < sectors; j++, k1++, k2++)
        {
            if(i != 0)
            {
                indices->push_back(k1);
                indices->push_back(k2);
                indices->push_back(k1 + 1);
            }

            if(i != (stacks - 1))
            {
                indices->push_back(k1 + 1);
                indices->push_back(k2);
                indices->push_back(k2 + 1);
            }
        }
    }
}

std::string phySim::readFile(const char* path)
{
    std::ifstream file(path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int phySim::testGL() {
    if (!glfwInit()) {
        std::cout << "Failed to init GLFW\n";
        return -1;
    }

    // Set OpenGL version (IMPORTANT)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Triangle", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window,frameBufferSizeCallBack);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to init GLAD\n";
        return -1;
    }

    // Print OpenGL version
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    // Triangle vertices
    float vertices[] = {
    // back face
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,

    // front face
    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,

    // left face
    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,

    // right face
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,

    // bottom face
    // -0.5f, -0.5f, -0.5f,
    //  0.5f, -0.5f, -0.5f,
    //  0.5f, -0.5f,  0.5f,
    //  0.5f, -0.5f,  0.5f,
    // -0.5f, -0.5f,  0.5f,
    // -0.5f, -0.5f, -0.5f,

    // top face
    // -0.5f,  0.5f, -0.5f,
    //  0.5f,  0.5f, -0.5f,
    //  0.5f,  0.5f,  0.5f,
    //  0.5f,  0.5f,  0.5f,glm::rotate(model, angle, glm::vec3(0,0,1));
    // -0.5f,  0.5f,  0.5f,
    // -0.5f,  0.5f, -0.5f
};

    float vertices_line[] = {
        //x axis
        0.0,0.0,0.0,
        1.0,0.0,0.0,
        //y axis
        0.0,0.0,0.0,
        0.0,1.0,0.0,
        //z axis
        0.0,0.0,0.0,
        0.0,0.0,1.0,


    };

    float dt = 0.1;
    float time = 0;
    float v = 0.5;
    float w = 0.3;
    float theta = 0;
    float qx = 0,qy = 0,qz = 0,qw = 0;
    float posx = 0,posy = 0,posz = 0;

    
    glm::mat4 model_axes = glm::mat4(1.0f); // identity (no rotation)
    // model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));

    view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f));

    projection = glm::perspective(glm::radians(45.0f),(float)800 / 600, 0.1f,100.0f);

    model = glm::mat4(1.0f);

    glm::vec3 tr_offset = glm::vec3(0,0,0);

    std::string vertexCode = readFile(R"(/home/srinidhi/Documents/Simulations/Testing/Project1/src/shaders/vertexShader.glsl)");

    // Create buffers
    unsigned int VAO, VBO,VAO2,VBO2;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glGenVertexArrays(1, &VAO2);
    glGenBuffers(1, &VBO2);

    // Bind VAO first
    glBindVertexArray(VAO);

    // Bind and fill VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Describe vertex layout
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(VAO2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_line), vertices_line, GL_STATIC_DRAW);

    // Describe vertex layout
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    

    // Compile vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexShaderSource = vertexCode.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Compile fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Cleanup shaders (no longer needed after linking)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    int quatLoc = glGetUniformLocation(shaderProgram, "quat");
    int offset = glGetUniformLocation(shaderProgram, "offset");
    int colorLoc = glGetUniformLocation(shaderProgram, "color");

    int modelLoc = glGetUniformLocation(shaderProgram, "model");
    int viewLoc  = glGetUniformLocation(shaderProgram, "view");
    int projLoc  = glGetUniformLocation(shaderProgram, "projection");

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // glfwSetScrollCallback(window, scroll_callback);
    // glfwSetMouseButtonCallback(window, mouse_button_callback);
    // glfwSetCursorPosCallback(window, cursor_position_callback);

    

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // checkExitStatus(window,GLFW_KEY_ESCAPE);
        // transformBody(window,&tr_offset,&view);

        glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // glUniform4f(quatLoc, qx, qy, qz, qw);
        // glUniform3f(offset,posz,posy,posz);
        

        // glUseProgram(shaderProgram);
        // glBindVertexArray(VAO);

        // glDrawArrays(GL_TRIANGLES, 0, 36);
        // glBindVertexArray(VAO2);

        // glDrawArrays(GL_LINE_STRIP, 0, 5);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        // glUniform3f(offset,tr_offset.x,tr_offset.y,tr_offset.z);

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        

        // 1. Draw filled cube
        glUniform3f(colorLoc,0.5,0.25,0.0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // 2. Draw edges on top
        glUniform3f(colorLoc,0.5,0.5,0.5);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(2.0f);

        // Optional: make edges always visible
        glDisable(GL_DEPTH_TEST);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        glEnable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glBindVertexArray(VAO2);
        glUniform3f(offset,0,0,0);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model_axes));
        glUniform3f(colorLoc,1,0.25,0.0);
        glLineWidth(2.0f);
        glDisable(GL_DEPTH_TEST);
        glDrawArrays(GL_LINES,0,6);
        glEnable(GL_DEPTH_TEST);
        time = time + dt;
        theta = w*dt;

        qw = cos(theta/2);
        qz = sin(theta/2);

        // posy = posy + v*dt;
        // if(posy > 10) {
        //     posy = -10;
        // }
        // model = glm::rotate(model, theta, glm::vec3(0.0f, 0.0f, 1.0f));
        float angle = glfwGetTime();

        // model = glm::translate(model, tr_offset);

        // model = glm::mat4(1.0f);
        // model = glm::rotate(model, angle, glm::vec3(0,0,1));
        // model = glm::translate(model,glm::vec3(posx,posy,posz));


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
}

void phySim::simLoop() {

    isDragging = false;
    firstMouse = true;

    up_vector = glm::vec3(0,1,0);
    right_vector = glm::vec3(1,0,0);
    forward_vector = glm::vec3(0,0,1);

    lastX = 0;
    lastY = 0;

    if (!glfwInit()) {
        std::cout << "Failed to init GLFW\n";
        return;
    }

    // Set OpenGL version (IMPORTANT)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    window = glfwCreateWindow(800, 600, "Triangle", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create window\n";
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window,frameBufferSizeCallBack);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to init GLAD\n";
        return;
    }

    Re_sim = 100000;
    Re_scale = Re_sim/Re;
    rocket_radius = 100;
    rocket_height = 1000;
    populateSphereVertices(Re_scale * Re,&earth_vertices,&earth_indices);
    // populateCylinderVertices(rocket_radius * Re_scale,rocket_height * Re_scale,&rocket_vertices,&rocket_indices);
    populateSphereVertices(rocket_radius * Re_scale,&rocket_vertices,&rocket_indices);


    std::cout << "Vertices: "
          << earth_vertices.size()
          << std::endl;

    std::cout << "Indices: "
            << earth_indices.size()
            << std::endl;

    glm::mat4 model_axes = glm::mat4(1.0f); // identity (no rotation)
    // model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));

    float camDistance = 20.0f * rocket_radius * Re_scale;

    glm::vec3 rocketPos(
        s.x * Re_scale,
        s.y * Re_scale,
        s.z * Re_scale
    );

    // Camera position
    glm::vec3 cameraPos =
        rocketPos + glm::vec3(0, camDistance, camDistance);

    // Look at rocket
    view = glm::lookAt(
        cameraPos,
        rocketPos,
        glm::vec3(1.0f, 0.0f, 0.0f)
    );

    projection = glm::perspective(
        glm::radians(45.0f),
        800.0f/600.0f,
        1.0f,
        (float)(10.0f * Re_sim)
    );

    fov = 45;

    model = glm::mat4(1.0f);

    model_rocket = glm::mat4(1.0f);
    model_rocket = glm::translate(model_rocket, glm::vec3(s.x * Re_scale, s.y * Re_scale, s.z * Re_scale));

    glm::vec3 tr_offset = glm::vec3(0,0,0);

    std::string vertexCode = readFile(R"(/home/srinidhi/Documents/Simulations/Orbital_Mechanics/CppSim/src/shaders/vertexShader.glsl)");

    glGenVertexArrays(1, &earth_vao);
    glGenBuffers(1, &earth_vbo);
    glGenBuffers(1, &earth_ebo);

    // glGenVertexArrays(1, &VAO2);
    // glGenBuffers(1, &VBO2);

    // Bind VAO first
    glBindVertexArray(earth_vao);

    // Bind and fill VBO
    glBindBuffer(GL_ARRAY_BUFFER, earth_vbo);
    glBufferData(GL_ARRAY_BUFFER, earth_vertices.size() * sizeof(float) , earth_vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, earth_ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        earth_indices.size() * sizeof(unsigned int),
        earth_indices.data(),
        GL_STATIC_DRAW
    );

    // Describe vertex layout
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glGenVertexArrays(1, &rocket_vao);
    glGenBuffers(1, &rocket_vbo);
    glGenBuffers(1, &rocket_ebo);

    // glGenVertexArrays(1, &VAO2);
    // glGenBuffers(1, &VBO2);

    // Bind VAO first
    glBindVertexArray(rocket_vao);

    // Bind and fill VBO
    glBindBuffer(GL_ARRAY_BUFFER, rocket_vbo);
    glBufferData(GL_ARRAY_BUFFER, rocket_vertices.size() * sizeof(float) , rocket_vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rocket_ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        rocket_indices.size() * sizeof(unsigned int),
        rocket_indices.data(),
        GL_STATIC_DRAW
    );

    // Describe vertex layout
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Compile vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexShaderSource = vertexCode.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Compile fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Cleanup shaders (no longer needed after linking)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    int quatLoc = glGetUniformLocation(shaderProgram, "quat");
    int offset = glGetUniformLocation(shaderProgram, "offset");
    int colorLoc = glGetUniformLocation(shaderProgram, "color");

    int modelLoc = glGetUniformLocation(shaderProgram, "model");
    int viewLoc  = glGetUniformLocation(shaderProgram, "view");
    int projLoc  = glGetUniformLocation(shaderProgram, "projection");

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    int epfd = epoll_create1(0);

    int tfd = timerfd_create(
        CLOCK_MONOTONIC,
        0
    );

    double time_scale = 1e2;

    itimerspec timer{};
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_nsec = dt * 1e9 / time_scale;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_nsec = dt * 1e9 / time_scale;

    timerfd_settime(
        tfd,
        0,
        &timer,
        nullptr
    );

    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = tfd;

    epoll_ctl(
        epfd,
        EPOLL_CTL_ADD,
        tfd,
        &ev
    );

    epoll_event event;

    auto now = std::chrono::system_clock::now();

    // 2. Convert to a legacy time_t structure
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

    // 3. Convert to local time structure
    std::tm* localTime = std::localtime(&currentTime);

    std::ostringstream logfilename;
    logfilename <<"logs/"<< std::put_time(localTime, "%Y_%m_%d_%H_%M_%S")
             << ".txt";

    std::cout<< "Log file name = " << logfilename.str() << std::endl;

    std::ofstream logfile(logfilename.str());

    setLogFile(&logfile);

    guidInit("input/Guid_Input_Params.txt");

    sim_iter = 1;

    glm::mat4 R;

    while (!glfwWindowShouldClose(window))
    {

        int n = epoll_wait(epfd,&event,1,0);
        if(n >= 1) {
            std::cout<<"iter no = " << sim_iter << std::endl;
            time = sim_iter * dt;
            updateLocalFrame(v,&local_FPA);
            g1->getHeading(s,Xv,&heading_angle);
            g1->getGuidanceOutput(s,v,local_FPA,time,&commanded_pitch,&isThrusting);
            updatePosition(commanded_pitch,heading_angle,isThrusting);
            model_rocket = glm::translate(model_rocket, glm::vec3(v.x * dt * Re_scale, v.y * dt * Re_scale, v.z * dt * Re_scale));
            rocketPos = glm::vec3(s.x * Re_scale, s.y * Re_scale, s.z * Re_scale);
            cameraPos = rocketPos + glm::vec3(0,camDistance,camDistance);
            view = glm::lookAt(
                cameraPos,
                rocketPos,
                glm::vec3(1.0f, 0.0f, 0.0f)
            );
            // view = glm::rotate(view, (float)(dy * 0.01), right_vector);
            // R = glm::rotate(glm::mat4(1.0f), (float)(dy * 0.01), right_vector);
            // up_vector = glm::vec3(R * glm::vec4(up_vector, 1.0f));
            // view = glm::rotate(view, (float)(dx * 0.01), up_vector);
            // R = glm::rotate(glm::mat4(1.0f), (float)(dx * 0.01), up_vector);
            // right_vector = glm::vec3(R * glm::vec4(right_vector, 1.0f));
            sim_iter += 1;
            std::cout<<"n = " << n << std::endl;
            if(time > stop_time) {
                std::cout<<"Finished simulation " << std::endl;
                break;
            }
            
        }
        checkExitStatus(window,GLFW_KEY_ESCAPE);
        transformBody(window,&tr_offset,&view);

        glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // glUniform4f(quatLoc, qx, qy, qz, qw);
        // glUniform3f(offset,posz,posy,posz);
        

        // glUseProgram(shaderProgram);
        // glBindVertexArray(VAO);

        // glDrawArrays(GL_TRIANGLES, 0, 36);
        // glBindVertexArray(VAO2);

        // glDrawArrays(GL_LINE_STRIP, 0, 5);

        glUseProgram(shaderProgram);
        glBindVertexArray(earth_vao);
        // glUniform3f(offset,tr_offset.x,tr_offset.y,tr_offset.z);

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        

        // 1. Draw filled cube
        glUniform3f(colorLoc,0.5,0.25,0.0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawElements(GL_TRIANGLES,static_cast<GLsizei>(earth_indices.size()),GL_UNSIGNED_INT,0);
        // glDrawArrays(GL_TRIANGLES, 0, 36);

        // 2. Draw edges on top
        glUniform3f(colorLoc,0.5,0.5,0.5);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(2.0f);

        // Optional: make edges always visible
        glDisable(GL_DEPTH_TEST);

        glDrawElements(GL_TRIANGLES,static_cast<GLsizei>(earth_indices.size()),GL_UNSIGNED_INT,0);
        // glDrawArrays(GL_TRIANGLES, 0, 36);
        // glDrawElements(GL_TRIANGLES,static_cast<GLsizei>(earth_indices.size()),GL_UNSIGNED_INT,0);

        glEnable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glBindVertexArray(rocket_vao);
        // glUniform3f(offset,tr_offset.x,tr_offset.y,tr_offset.z);

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model_rocket));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        

        // 1. Draw filled cube
        glUniform3f(colorLoc,1.0,0.0,0.0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        // glDrawArrays(GL_TRIANGLES, 0, 36);

        // 2. Draw edges on top
        glUniform3f(colorLoc,1.0,0.0,0.0);
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(2.0f);

        // Optional: make edges always visible
        // glDisable(GL_DEPTH_TEST);

        // glDrawArrays(GL_TRIANGLES, 0, 36);
        glDrawElements(GL_TRIANGLES,static_cast<GLsizei>(rocket_indices.size()),GL_UNSIGNED_INT,0);

        glEnable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // glBindVertexArray(VAO2);
        // glUniform3f(offset,0,0,0);
        // glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model_axes));
        // glUniform3f(colorLoc,1,0.25,0.0);
        // glLineWidth(2.0f);
        // glDisable(GL_DEPTH_TEST);
        // glDrawArrays(GL_LINES,0,6);
        // glEnable(GL_DEPTH_TEST);
        // time = time + dt;
        // theta = w*dt;

        // qw = cos(theta/2);
        // qz = sin(theta/2);

        // posy = posy + v*dt;
        // if(posy > 10) {
        //     posy = -10;
        // }
        // model = glm::rotate(model, theta, glm::vec3(0.0f, 0.0f, 1.0f));
        float angle = glfwGetTime();

        // model = glm::translate(model, tr_offset);

        // model = glm::mat4(1.0f);
        // model = glm::rotate(model, angle, glm::vec3(0,0,1));
        // model = glm::translate(model,glm::vec3(posx,posy,posz));

        // std::cout << window << std::endl;
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR)
        {
            std::cout << "OpenGL error: " << err << std::endl;
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &earth_vao);
    glDeleteBuffers(1, &earth_vbo);
    glDeleteBuffers(1, &earth_ebo);
    glfwDestroyWindow(window);

    glfwTerminate();

}