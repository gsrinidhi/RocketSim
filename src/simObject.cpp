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

    glBindVertexArray(0);
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