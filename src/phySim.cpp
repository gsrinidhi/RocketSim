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
    simMode = configMap["Mode"];

    mass = Mfo + Ms + Mp;
    Mf = Mfo;

    g_mag = GRAVITATIONAL_CONSTANT * Me/(Re*Re);
    depletion_flag = 0;

    double sxo = Re*sin(phi_init)*cos(theta_init);
    double syo = Re*sin(phi_init)*sin(theta_init);
    double szo = Re*cos(phi_init);
    s.setXYZ(sxo, syo, szo); // Initial position at Earth's surface
    v.setXYZ(0,0,0);

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

void phySim::updatePositionQuat(Quaternion cmd_q,double isThrusting) {
    double r = s.magnitude();
    g_local = GRAVITATIONAL_CONSTANT * Me / (r * r);
    if(isThrusting && depletion_flag == 0) {
        thrustMag = m * Isp * g_mag; // Update thrust magnitude based on current mass flow rate
    } else {
        thrustMag = 0; // No thrust during coasting
    }
    //Thrust axis is along body x axis. Rotate inertial X axis by the command quaternion to get thrust direction in inertial frame
    //get conjugate of command quaternion
    Quaternion cmd_q_conj = cmd_q.conjugate();
    //Rotate inertial X axis by command quaternion to get thrust direction in inertial frame
    phyVector thrust_direction = cmd_q.rotateVector(phyVector(1, 0, 0)); // Rotate the local X-axis by the command quaternion
    a = thrust_direction * (thrustMag/mass) - Zv * g_local; // Acceleration due to thrust and gravity
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

void phySim::getLocalFrame(phyVector *Xv,phyVector *Yv,phyVector *Zv) {
    *Xv = this->Xv;
    *Yv = this->Yv;
    *Zv = this->Zv;
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
    windowWidth = width;
    windowHeight = height;
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

    // dx = (xpos - lastX) * Re_scale;
    // dy = (ypos - lastY) * Re_scale;
    dy = (ypos - lastY);
    double theta = -dy * 0.01;
    cam_quat = Quaternion(theta, rightVector);
    forwardVector = cam_quat.rotateVector(forwardVector);
    camPos = s * Re_scale + forwardVector * camDistance;
    upVector = cam_quat.rotateVector(upVector);
    // view = glm::lookAt(
    //                 glm::vec3(camPos.x, camPos.y, camPos.z),
    //                 glm::vec3(s.x * Re_scale, s.y * Re_scale, s.z * Re_scale),
    //                 glm::vec3(upVector.x, upVector.y, upVector.z)
    //             );
    dx = (xpos - lastX);
    theta = -dx * 0.01;
    cam_quat = Quaternion(theta, upVector);
    forwardVector = cam_quat.rotateVector(forwardVector);
    camPos = s * Re_scale + forwardVector * camDistance;
    rightVector = cam_quat.rotateVector(rightVector);
    view = glm::lookAt(
                    glm::vec3(camPos.x, camPos.y, camPos.z),
                    glm::vec3(s.x * Re_scale, s.y * Re_scale, s.z * Re_scale),
                    glm::vec3(upVector.x, upVector.y, upVector.z)
                );

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

void phySim::checkSymStart(GLFWwindow* window,int key) {
    if(glfwGetKey(window,key) == GLFW_PRESS) {
        sim_run_flag = 1;
    }
}

void phySim::checkSymStop(GLFWwindow* window,int key) {
    if(glfwGetKey(window,key) == GLFW_PRESS) {
        sim_run_flag = 0;
    }
}

void phySim::transformBody(GLFWwindow *window,glm::vec3 *offset,glm::mat4 *view ) {
    // float posx,poy,posz;
    // if(glfwGetKey(window,GLFW_KEY_UP) == GLFW_PRESS) {
    //     offset->y = offset->y + 0.02;
    //     model = glm::translate(model, glm::vec3(0.0f, 0.02f, 0.0f));
    // } else if(glfwGetKey(window,GLFW_KEY_DOWN) == GLFW_PRESS) {
    //     offset->y = offset->y - 0.02;
    //     model = glm::translate(model, glm::vec3(0.0f, -0.02f, 0.0f));
    // } else if(glfwGetKey(window,GLFW_KEY_RIGHT) == GLFW_PRESS) {
    //     offset->x = offset->x + 0.02;
    //     model = glm::translate(model, glm::vec3(0.02f * Re * Re_scale, 0.0f, 0.0f));
    // } else if(glfwGetKey(window,GLFW_KEY_LEFT) == GLFW_PRESS) {
    //     offset->x = offset->x - 0.02;
    //     model = glm::translate(model, glm::vec3(-0.02f * Re * Re_scale, 0.0f, 0.0f));
    // } else if(glfwGetKey(window,GLFW_KEY_W) == GLFW_PRESS) {
    //     *view = glm::translate(*view, glm::vec3(0.0f, 0.2f * Re * Re_scale, 0.0f));
    // } else if(glfwGetKey(window,GLFW_KEY_S) == GLFW_PRESS) {
    //     *view = glm::translate(*view, glm::vec3(0.0f, -0.2f * Re * Re_scale, 0.0f));
    // } else if(glfwGetKey(window,GLFW_KEY_A) == GLFW_PRESS) {
    //     *view = glm::translate(*view, glm::vec3(-0.2f * Re * Re_scale, 0.0f, 0.0f));
    // } else if(glfwGetKey(window,GLFW_KEY_D) == GLFW_PRESS) {
    //     *view = glm::translate(*view, glm::vec3(0.2f * Re * Re_scale, 0.0f, 0.0f));
    // } else if(glfwGetKey(window,GLFW_KEY_R) == GLFW_PRESS) {
    //     *view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f * Re * Re_scale));
    //     up_vector = glm::vec3(0,1,0);
    //     right_vector = glm::vec3(1,0,0);
    //     forward_vector = glm::vec3(0,0,1);
    // } 
}

int phySim::initRender() {
    isDragging = false;
    firstMouse = true;

    up_vector = glm::vec3(0,1,0);
    right_vector = glm::vec3(1,0,0);
    forward_vector = glm::vec3(0,0,1);

    lastX = 0;
    lastY = 0;

    if (!glfwInit()) {
        std::cout << "Failed to init GLFW\n";
        return -1;
    }

    // Set OpenGL version (IMPORTANT)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    windowWidth = 800;
    windowHeight = 600;

    // Create window
    window = glfwCreateWindow(windowWidth, windowHeight, "Triangle", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create window\n";
        glfwTerminate();
        return -1; 
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window,frameBufferSizeCallBack);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to init GLAD\n";
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

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

void phySim::populateCylinderVertices(double radius,double height,SimObject *obj) {
    int sectors = 36;
    int stacks = 18;

    for(int i = 0; i <= stacks; i++)
    {
        // float stackAngle = M_PI/2 - i * M_PI/stacks;
        // float xy = radius * cos(stackAngle);
        float hz  = height * i/stacks;

        for(int j = 0; j <= sectors; j++)
        {
            float sectorAngle = j * 2*M_PI/sectors;

            float x = obj->getBodyFrameX().x * hz + obj->getBodyFrameZ().x * radius * cos(sectorAngle) + obj->getBodyFrameY().x * radius * sin(sectorAngle);
            float y = obj->getBodyFrameX().y * hz + obj->getBodyFrameZ().y * radius * cos(sectorAngle) + obj->getBodyFrameY().y * radius * sin(sectorAngle);
            float z = obj->getBodyFrameX().z * hz + obj->getBodyFrameZ().z * radius * cos(sectorAngle) + obj->getBodyFrameY().z * radius * sin(sectorAngle);

            obj->addVertex(x,y,z);
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
                obj->addIndex(k1);
                obj->addIndex(k2);
                obj->addIndex(k1 + 1);
            }

            if(i != (stacks - 1))
            {
                obj->addIndex(k1 + 1);
                obj->addIndex(k2);
                obj->addIndex(k2 + 1);
            }
        }
    }
}

void phySim::populateConeVertices(double radius,double height,SimObject *obj) {
    int sectors = 36;
    int stacks = 18;

    for(int i = 0; i <= stacks; i++)
    {
        // float stackAngle = M_PI/2 - i * M_PI/stacks;
        // float xy = radius * cos(stackAngle);
        float hz  = height * i/stacks;

        float rc = radius/height * (height - hz); // radius of the cone at height hz

        for(int j = 0; j <= sectors; j++)
        {
            float sectorAngle = j * 2*M_PI/sectors;

            float x = obj->getBodyFrameX().x * hz + obj->getBodyFrameZ().x * rc * cos(sectorAngle) + obj->getBodyFrameY().x * rc * sin(sectorAngle);
            float y = obj->getBodyFrameX().y * hz + obj->getBodyFrameZ().y * rc * cos(sectorAngle) + obj->getBodyFrameY().y * rc * sin(sectorAngle);
            float z = obj->getBodyFrameX().z * hz + obj->getBodyFrameZ().z * rc * cos(sectorAngle) + obj->getBodyFrameY().z * rc * sin(sectorAngle);

            obj->addVertex(x,y,z);
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
                obj->addIndex(k1);
                obj->addIndex(k2);
                obj->addIndex(k1 + 1);
            }

            if(i != (stacks - 1))
            {
                obj->addIndex(k1 + 1);
                obj->addIndex(k2);
                obj->addIndex(k2 + 1);
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
    return 0;
}

void phySim::simLoop() {

    int initRenderStatus = initRender();
    if(initRenderStatus != 0) {
        std::cout << "Failed to init render\n";
        return;
    }

    Re_sim = 100000;
    Re_scale = Re_sim/Re;
    rocket_radius = 100;
    rocket_height = 1000;
    SimBody rocket_body;
    SimObject rocket_cylinder,rocket_cone,earth,rocket_plume,fin1,fin2;
    SimObject axis_x,axis_y,axis_z;
    axis_x.setPosition(phyVector(0, 0, 0));
    axis_y.setPosition(phyVector(0, 0, 0));
    axis_z.setPosition(phyVector(0, 0, 0));

    rocket_cylinder.setPosition(phyVector(s.x , s.y, s.z));
    rocket_cylinder.setBodyFrameAxes(Zv,-Yv,Xv);
    rocket_body.setPosition(phyVector(s.x , s.y, s.z));
    rocket_body.setVelocity(phyVector(0,0,0));
    rocket_cone.setPosition(phyVector((s.x + rocket_height) , s.y , s.z ));
    rocket_cone.setBodyFrameAxes(Zv,-Yv,Xv);
    earth.setPosition(phyVector(0, 0, 0));
    earth.setBodyFrameAxes(phyVector(1, 0, 0),phyVector(0, 1, 0),phyVector(0, 0, 1));
    rocket_plume.setPosition(phyVector((s.x - rocket_height/15) , s.y , s.z ));
    rocket_plume.setBodyFrameAxes(Zv,-Yv,Xv);
    fin1.setPosition(phyVector(s.x , (s.y + rocket_radius*1.1) , s.z ));
    fin2.setPosition(phyVector(s.x , (s.y - rocket_radius*1.1) , s.z ));
    fin1.setBodyFrameAxes(Zv,-Yv,Xv);
    fin2.setBodyFrameAxes(Zv,-Yv,Xv);
    populateSphereVertices(Re_scale * Re,&earth_vertices,&earth_indices);
    earth.setVerticesAndIndices(earth_vertices,earth_indices);
    populateCylinderVertices(rocket_radius * Re_scale,rocket_height * Re_scale,&rocket_cylinder);
    populateConeVertices(rocket_radius * Re_scale,rocket_height/10 * Re_scale,&rocket_cone);
    populateConeVertices(rocket_radius * Re_scale,rocket_height/10 * Re_scale,&rocket_plume);
    populateConeVertices(rocket_radius/10 * Re_scale,rocket_height/10 * Re_scale,&fin1);
    populateConeVertices(rocket_radius/10 * Re_scale,rocket_height/10 * Re_scale,&fin2);
    earth.setColor(0.0, 0.0, 1.0); // Blue color
    earth.initOpenGLBuffers();
    rocket_cylinder.setColor(1.0, 0.0, 0.0); // Red color
    rocket_cylinder.initOpenGLBuffers();
    rocket_cone.setColor(0.0, 1.0, 0.0); // Green color
    rocket_cone.initOpenGLBuffers();
    rocket_plume.setColor(1.0, 1.0, 0.0); // Yellow color
    rocket_plume.initOpenGLBuffers();
    fin1.setColor(0.5, 0.5, 0.5); // Gray color
    fin1.initOpenGLBuffers();
    fin2.setColor(0.5, 0.5, 0.5); // Gray color
    fin2.initOpenGLBuffers();

    std::cout << "Vertices: "
          << earth_vertices.size()
          << std::endl;

    std::cout << "Indices: "
            << earth_indices.size()
            << std::endl;

    glm::mat4 model_axes = glm::mat4(1.0f); // identity (no rotation)

    camDistance = 20.0f * rocket_radius * Re_scale;

    glm::vec3 rocketPos(
        s.x * Re_scale,
        s.y * Re_scale,
        s.z * Re_scale
    );

    // Camera position

    camoffset = phyVector(0, camDistance, camDistance);
    camPos = s + camoffset;
    forwardVector = camoffset/camoffset.magnitude();
    upVector.setXYZ(1, 0, 0);
    rightVector = upVector ^ forwardVector;
    glm::vec3 cameraPos =
        rocketPos + glm::vec3(camoffset.x, camoffset.y, camoffset.z);

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

    earth.setLoc(modelLoc,viewLoc,projLoc,colorLoc,offset);
    rocket_plume.setLoc(modelLoc,viewLoc,projLoc,colorLoc,offset);
    fin1.setLoc(modelLoc,viewLoc,projLoc,colorLoc,offset);
    fin2.setLoc(modelLoc,viewLoc,projLoc,colorLoc,offset);
    rocket_cylinder.setLoc(modelLoc,viewLoc,projLoc,colorLoc,offset);
    rocket_cone.setLoc(modelLoc,viewLoc,projLoc,colorLoc,offset);
    // rocket_plume.setLoc(modelLoc,viewLoc,projLoc,colorLoc,offset);

    phyVector offset1(0, 0, 0);
    phyVector offset2(rocket_height , 0, 0);
    phyVector offset3(-rocket_height/15 , 0, 0);
    phyVector offset4(0, rocket_radius * 1.1 , 0);
    phyVector offset5(0, -rocket_radius * 1.1 , 0);
    rocket_body.addObject(rocket_cylinder, offset1);
    rocket_body.addObject(rocket_cone, offset2);
    rocket_body.addObject(rocket_plume, offset3);
    rocket_body.addObject(fin1, offset4);
    rocket_body.addObject(fin2, offset5);
    rocket_body.setLoc(modelLoc,viewLoc,projLoc,colorLoc,offset);
    rocket_body.setBodyFrameAxes(Zv,-Yv,Xv);

    int epfd = epoll_create1(0);

    int tfd = timerfd_create(
        CLOCK_MONOTONIC,
        0
    );

    double time_scale = 10;

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

    sim_iter = 0;

    glm::mat4 R;

    sim_run_flag = 0;

    Quaternion cmd_q,cmd_q_conj;

    double rotate_angle = 0;
    phyVector rot_axis(0, 0, 1);
    std::vector<int> drawableFlags;
    drawableFlags.push_back(1);
    drawableFlags.push_back(1);
    drawableFlags.push_back(1);
    earth.setDrawLines(1);

    double apogee =0,perigee = 0;

    

    while (!glfwWindowShouldClose(window))
    {

        checkSymStart(window,GLFW_KEY_S);
        checkSymStop(window,GLFW_KEY_P);
        glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        if(sim_iter == 0) {
            // glClearColor(0,0,0,1);
            // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // glUseProgram(shaderProgram);
            earth.draw(view,projection,model,shaderProgram);
            // rocket_body.draw(view,projection,model_rocket,shaderProgram);
            rocket_body.drawUsingState(Re_scale,view,projection,shaderProgram);
            glfwSwapBuffers(window);
            sim_iter += 1;
            
        }
        if(sim_run_flag == 1) {
            int n = epoll_wait(epfd,&event,1,0);
            if(n >= 1) {
                std::cout<<"iter no = " << sim_iter << std::endl;
                time = sim_iter * dt;
                updateLocalFrame(v,&local_FPA);
                g1->getHeading(s,Xv,&heading_angle);
                // g1->getGuidanceOutput(s,v,local_FPA,time,&commanded_pitch,&isThrusting);
                g1->getGuidanceOutputQuat(s,v,local_FPA,time,heading_angle,Xv,Yv,Zv,cmd_q,&isThrusting);
                // updatePosition(commanded_pitch,heading_angle,isThrusting);
                updatePositionQuat(cmd_q,isThrusting);
                g1->getApoapsisPeriapsis(&apogee,&perigee);
                cmd_q_conj = cmd_q.conjugate();
                // rotate_angle = cmd_q_conj.getRotationAngle();
                // rot_axis = cmd_q_conj.getRotationAxis();
                rotate_angle = cmd_q.getRotationAngle();
                rot_axis = cmd_q.getRotationAxis();
                model_rocket = glm::mat4(1.0f);
                
                // model_rocket = glm::translate(model_rocket, glm::vec3(v.x * dt * Re_scale, v.y * dt * Re_scale, v.z * dt * Re_scale));
                model_rocket = glm::translate(model_rocket, glm::vec3(s.x * Re_scale, s.y * Re_scale, s.z * Re_scale));
                model_rocket = glm::rotate(model_rocket, (float)rotate_angle, glm::vec3(rot_axis.x,rot_axis.y,rot_axis.z));
                camoffset = forwardVector * camDistance;
                rocketPos = glm::vec3(s.x * Re_scale, s.y * Re_scale, s.z * Re_scale);
                cameraPos = rocketPos + glm::vec3(camoffset.x,camoffset.y,camoffset.z);
                view = glm::lookAt(
                    cameraPos,
                    rocketPos,
                    glm::vec3(upVector.x, upVector.y, upVector.z)
                );
                if(isThrusting == 1) {
                    drawableFlags[2] = 1;
                } else {
                    drawableFlags[2] = 0;
                }
                rocket_body.updateDrawableFlags(drawableFlags);
                sim_iter += 1;
                std::cout<<"n = " << n << std::endl;
                if(time > stop_time) {
                    std::cout<<"Finished simulation " << std::endl;
                    break;
                }

                // transformBody(window,&tr_offset,&view);

                
                rocket_body.updateState(cmd_q,a,dt,Re_scale);



                
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(windowWidth/5, windowHeight/5));

        ImGui::Begin("Telemetry");

        ImGui::Text("Time: %.2f", time);
        ImGui::Text("Altitude: %.2f km", (s.magnitude() - Re)/1000);
        ImGui::Text("Velocity: %.2f m/s", v.magnitude());
        ImGui::Text("Apogee: %.2f km", apogee/1000);
        ImGui::Text("Perigee: %.2f km", perigee/1000);

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        earth.draw(view,projection,model,shaderProgram);

        // rocket_body.draw(view,projection,model_rocket,shaderProgram);
        
        rocket_body.drawUsingState(Re_scale,view,projection,shaderProgram);

        

        glfwSwapBuffers(window);
                
        
        checkExitStatus(window,GLFW_KEY_ESCAPE);
        
        glfwPollEvents();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glDeleteVertexArrays(1, &earth_vao);
    glDeleteBuffers(1, &earth_vbo);
    glDeleteBuffers(1, &earth_ebo);
    glfwDestroyWindow(window);

    glfwTerminate();

}