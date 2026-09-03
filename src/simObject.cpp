#include "simObject.hpp"

SimObject::SimObject() : position(0, 0, 0), velocity(0, 0, 0), acceleration(0, 0, 0), mass(1.0) {
    body_x = phyVector(1, 0, 0);
    body_y = phyVector(0, 1, 0);
    body_z = phyVector(0, 0, 1);
}

SimObject::SimObject(phyVector position, phyVector velocity, phyVector acceleration, double mass)
    : position(position), velocity(velocity), acceleration(acceleration), mass(mass) {
    body_x = phyVector(1, 0, 0);
    body_y = phyVector(0, 1, 0);
    body_z = phyVector(0, 0, 1);
}

void SimObject::setPosition(phyVector position) {
    this->position = position;
}

void SimObject::setVelocity(phyVector velocity) {
    this->velocity = velocity;
}

void SimObject::setAcceleration(phyVector acceleration) {
    this->acceleration = acceleration;
}

void SimObject::setMass(double mass) {
    this->mass = mass;
}

phyVector SimObject::getPosition() const {
    return position;
}

phyVector SimObject::getVelocity() const {
    return velocity;
}

phyVector SimObject::getAcceleration() const {
    return acceleration;
}

double SimObject::getMass() const {
    return mass;
}

void SimObject::setVerticesAndIndices(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {
    this->vertices = vertices;
    this->indices = indices;
}

void SimObject::setBodyFrameAxes(const phyVector& x, const phyVector& y, const phyVector& z) {
    this->body_x = x;
    this->body_y = y;
    this->body_z = z;
    rot_angle = 0;
    rot_axis.setXYZ(1,0,0);
}

void SimObject::initOpenGLBuffers() {
    // Initialize OpenGL buffers (VAO, VBO, EBO) for rendering the object
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Assuming the vertex data consists of positions only (3 floats per vertex)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    drawLines = 0; // Default to not drawing lines
}

void SimObject::setLoc(int modelLoc, int viewLoc, int projectionLoc,int colorLoc,int offsetLoc) {
    this->modelLoc = modelLoc;
    this->viewLoc = viewLoc;
    this->projectionLoc = projectionLoc;
    this->colorLoc = colorLoc;
    this->offsetLoc = offsetLoc;
}

void SimObject::setColor(double r, double g, double b) {
    color[0] = r;
    color[1] = g;
    color[2] = b;
}

void SimObject::draw(const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model, GLuint shaderProgram) {
    // glUseProgram(shaderProgram);
    glBindVertexArray(vao);

    // Set the uniform matrices for the shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Draw the object using the indices
    glUniform3f(colorLoc, color[0], color[1], color[2]);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

    if(this->drawLines) {
        glUniform3f(colorLoc, 1.0, 1.0, 1.0); // Set line color to white
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(2.0f);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
}

void SimObject::setDrawLines(int drawLines) {
    this->drawLines = drawLines;
}

phyVector SimObject::getBodyFrameX() {
    return body_x;
}

phyVector SimObject::getBodyFrameY() {
    return body_y;
}

phyVector SimObject::getBodyFrameZ() {
    return body_z;
}

void SimObject::addVertex(float x, float y, float z) {
    vertices.push_back(x);
    vertices.push_back(y);
    vertices.push_back(z);
}

void SimObject::addIndex(unsigned int index) {
    indices.push_back(index);
}

void SimObject::updateState(Quaternion cmd_q, phyVector acceleration,double dt,double scale,phyVector offset) {
    // Update the orientation of the object based on the provided quaternion
    body_x = cmd_q.rotateVector(phyVector(1, 0, 0));
    body_y = cmd_q.rotateVector(phyVector(0, 1, 0));
    body_z = cmd_q.rotateVector(phyVector(0, 0, 1));
    velocity = velocity + acceleration * dt;
    position = position + velocity * dt + offset;
    rot_angle = cmd_q.getRotationAngle();
    rot_axis = cmd_q.getRotationAxis();
}

void SimObject::updateState(Quaternion cmd_q, phyVector posRef,double dt,phyVector offset) {
    // Update the orientation of the object based on the provided quaternion
    body_x = cmd_q.rotateVector(phyVector(1, 0, 0));
    body_y = cmd_q.rotateVector(phyVector(0, 1, 0));
    body_z = cmd_q.rotateVector(phyVector(0, 0, 1));
    velocity = (posRef + offset - position)/dt;
    position = posRef + offset;
    // velocity = velocity + acceleration * dt;
    // position = position + velocity * dt + offset;
    rot_angle = cmd_q.getRotationAngle();
    rot_axis = cmd_q.getRotationAxis();
}


void SimObject::drawUsingState(double scale,const glm::mat4& view, const glm::mat4& projection, GLuint shaderProgram) {
    glm::mat4 R = glm::mat4(1.0f);
    // R[0] = glm::vec4(body_x.x,body_x.y,body_x.z,0.0f);
    // R[1] = glm::vec4(body_y.x,body_y.y,body_y.z,0.0f);
    // R[2] = glm::vec4(body_z.x,body_z.y,body_z.z,0.0f);
    // R[3] = glm::vec4(position.x * scale, position.y * scale, position.z * scale,1.0f);
    R = glm::translate(R, glm::vec3(position.x * scale, position.y * scale, position.z * scale));
    R = glm::rotate(R, (float)rot_angle, glm::vec3(rot_axis.x,rot_axis.y,rot_axis.z));
    draw(view,projection,R,shaderProgram);
}

void SimObject::iterateFunction(void *args) {
    // Placeholder for the iterate function implementation
    // This function can be used to perform iterative updates or calculations on the SimObject
}