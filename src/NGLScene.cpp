#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/Transformation.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOFactory.h>
#include <iostream>

NGLScene::NGLScene()
{
  setTitle("Klein Bottle taken from http://paulbourke.net/geometry/klein/");
}

NGLScene::~NGLScene()
{
  std::cout << "Shutting down NGL, removing VAO's and Shaders\n";
}

void NGLScene::resizeGL(int _w, int _h)
{
  m_project = ngl::perspective(45.0f, static_cast<float>(_w) / _h, 0.05f, 350.0f);
  m_win.width = static_cast<int>(_w * devicePixelRatio());
  m_win.height = static_cast<int>(_h * devicePixelRatio());
}

void NGLScene::initializeGL()
{
  // we must call this first before any other GL commands to load and link the
  // gl commands from the lib, if this is not done program will crash
  ngl::NGLInit::initialize();
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glClearColor(0.4f, 0.4f, 0.4f, 1.0f); // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);

  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(0, 12, 50);
  ngl::Vec3 to(0, 0, 0);
  ngl::Vec3 up(0, 1, 0);
  m_view = ngl::lookAt(from, to, up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_project = ngl::perspective(45.0f, 720.0f / 576.0f, 0.05f, 350.0f);

  ngl::ShaderLib::createShaderProgram("Phong");

  ngl::ShaderLib::attachShader("SimpleVertex", ngl::ShaderType::VERTEX);
  ngl::ShaderLib::attachShader("SimpleFragment", ngl::ShaderType::FRAGMENT);
  ngl::ShaderLib::loadShaderSource("SimpleVertex", "shaders/PhongVertex.glsl");
  ngl::ShaderLib::loadShaderSource("SimpleFragment", "shaders/PhongFragment.glsl");

  ngl::ShaderLib::compileShader("SimpleVertex");
  ngl::ShaderLib::compileShader("SimpleFragment");
  ngl::ShaderLib::attachShaderToProgram("Phong", "SimpleVertex");
  ngl::ShaderLib::attachShaderToProgram("Phong", "SimpleFragment");

  ngl::ShaderLib::linkProgramObject("Phong");
  ngl::ShaderLib::use("Phong");
  ngl::ShaderLib::setUniform("viewerPos", from);
  ngl::ShaderLib::setUniform("Normalize", 0);
  ngl::Vec4 lightPos(2.0f, 2.0f, 2.0f, 1.0f);
  ngl::Mat4 iv = m_view;
  iv.inverse().transpose();
  ngl::ShaderLib::setUniform("light.position", lightPos * iv);
  ngl::ShaderLib::setUniform("light.ambient", 0.1f, 0.1f, 0.1f, 1.0f);
  ngl::ShaderLib::setUniform("light.diffuse", 1.0f, 1.0f, 1.0f, 1.0f);
  ngl::ShaderLib::setUniform("light.specular", 0.8f, 0.8f, 0.8f, 1.0f);
  // gold like phong material
  ngl::ShaderLib::setUniform("material.ambient", 0.274725f, 0.1995f, 0.0745f, 0.0f);
  ngl::ShaderLib::setUniform("material.diffuse", 0.75164f, 0.60648f, 0.22648f, 0.0f);
  ngl::ShaderLib::setUniform("material.specular", 0.628281f, 0.555802f, 0.3666065f, 0.0f);
  ngl::ShaderLib::setUniform("material.shininess", 51.2f);
  createKleinBottle();
}

/// @brief
/// the following two function have been adapted from the following
/// website
/// http://paulbourke.net/geometry/klein/

ngl::Vec3 NGLScene::eval(float u, float v)
{
  ngl::Vec3 p;
  float r;

  r = 4.0f * (1.0f - cosf(u) / 2.0f);
  if (u < M_PI)
  {
    p.m_x = 6.0f * cosf(u) * (1.0f + sinf(u)) + r * cosf(u) * cosf(v);
    p.m_y = 16.0f * sinf(u) + r * sinf(u) * cosf(v);
  }
  else
  {
    p.m_x = 6.0f * cosf(u) * (1.0f + sinf(u)) + r * cosf(v + M_PI);
    p.m_y = 16.0f * sinf(u);
  }
  p.m_z = r * sinf(v);

  return p;
}

struct vertData
{
  GLfloat x;
  GLfloat y;
  GLfloat z;
  GLfloat nx;
  GLfloat ny;
  GLfloat nz;
  GLfloat u;
  GLfloat v;
};
void NGLScene::createKleinBottle()
{
  double umin = 0, umax = ngl::TWO_PI, vmin = 0, vmax = ngl::TWO_PI;
  int i, j, N;
  double u, v, dudv = 0.01;
  ngl::Vec3 p[4];
  ngl::Vec3 n[4];
  // a simple structure to hold our vertex data

  std::vector<vertData> data;

  N = 40;

  for (i = 0; i < N; ++i)
  {
    for (j = 0; j < N; ++j)
    {
      u = umin + i * (umax - umin) / (double)N;
      v = vmin + j * (vmax - vmin) / (double)N;
      p[0] = eval(u, v);
      n[0] = ngl::calcNormal(p[0], eval(u + dudv, v), eval(u, v + dudv));

      u = umin + (i + 1) * (umax - umin) / (double)N;
      v = vmin + j * (vmax - vmin) / (double)N;
      p[1] = eval(u, v);
      n[1] = ngl::calcNormal(p[1], eval(u + dudv, v), eval(u, v + dudv));

      u = umin + (i + 1) * (umax - umin) / (double)N;
      v = vmin + (j + 1) * (vmax - vmin) / (double)N;
      p[2] = eval(u, v);
      n[2] = ngl::calcNormal(p[2], eval(u + dudv, v), eval(u, v + dudv));

      u = umin + i * (umax - umin) / (double)N;
      v = vmin + (j + 1) * (vmax - vmin) / (double)N;
      p[3] = eval(u, v);
      n[3] = ngl::calcNormal(p[3], eval(u + dudv, v), eval(u, v + dudv));

      /* Write the geometry */

      vertData d;
      // tri 1

      d.x = p[0].m_x;
      d.y = p[0].m_y;
      d.z = p[0].m_z;

      d.nx = -n[0].m_x;
      d.ny = -n[0].m_y;
      d.nz = -n[0].m_z;
      data.push_back(d);

      d.x = p[1].m_x;
      d.y = p[1].m_y;
      d.z = p[1].m_z;

      d.nx = -n[1].m_x;
      d.ny = -n[1].m_y;
      d.nz = -n[1].m_z;
      data.push_back(d);

      d.x = p[2].m_x;
      d.y = p[2].m_y;
      d.z = p[2].m_z;

      d.nx = -n[2].m_x;
      d.ny = -n[2].m_y;
      d.nz = -n[2].m_z;
      data.push_back(d);

      /// tri 2
      d.x = p[2].m_x;
      d.y = p[2].m_y;
      d.z = p[2].m_z;

      d.nx = -n[2].m_x;
      d.ny = -n[2].m_y;
      d.nz = -n[2].m_z;
      data.push_back(d);

      d.x = p[3].m_x;
      d.y = p[3].m_y;
      d.z = p[3].m_z;

      d.nx = -n[3].m_x;
      d.ny = -n[3].m_y;
      d.nz = -n[3].m_z;
      data.push_back(d);

      d.x = p[0].m_x;
      d.y = p[0].m_y;
      d.z = p[0].m_z;

      d.nx = -n[0].m_x;
      d.ny = -n[0].m_y;
      d.nz = -n[0].m_z;
      data.push_back(d);
    }
  }
  m_vao = ngl::VAOFactory::createVAO("simpleVAO", GL_TRIANGLES);
  m_vao->bind();
  auto buffSize = data.size();
  // now we have our data add it to the VAO, we need to tell the VAO the following
  // how much (in bytes) data we are copying
  // a pointer to the first element of data (in this case the address of the first element of the
  // std::vector
  m_vao->setData(ngl::SimpleVAO::VertexData(buffSize * sizeof(vertData), data[0].x));

  // attribute vec3 inVert; attribute 0
  // attribute vec3 inNormal; attribute 1
  // attribute vec2 inUV; attribute 2
  m_vao->setVertexAttributePointer(0, 3, GL_FLOAT, sizeof(vertData), 0);
  // uv same as above but starts at 0 and is attrib 1 and only u,v so 2
  m_vao->setVertexAttributePointer(1, 3, GL_FLOAT, sizeof(vertData), 3);
  // normal same as vertex only starts at position 2 (u,v)-> nx
  m_vao->setVertexAttributePointer(2, 2, GL_FLOAT, sizeof(vertData), 6);
  // now we have set the vertex attributes we tell the VAO class how many indices to draw when
  // glDrawArrays is called, in this case we use buffSize (but if we wished less of the sphere to be drawn we could
  // specify less (in steps of 3))
  m_vao->setNumIndices(buffSize);
  // finally we have finished for now so time to unbind the VAO
  m_vao->unbind();
}
/// End of citation

void NGLScene::paintGL()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, m_win.width, m_win.height);
  ngl::ShaderLib::use("Phong");
  // clear the screen and depth buffer
  ngl::Transformation trans;
  // Rotation based on the mouse position for our global
  // transform
  ngl::Mat4 rotX = ngl::Mat4::rotateX(m_win.spinXFace);
  ngl::Mat4 rotY = ngl::Mat4::rotateY(m_win.spinYFace);
  // multiply the rotations
  m_mouseGlobalTX = rotY * rotX;
  // add the translations
  m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
  m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
  m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;

  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  M = m_mouseGlobalTX;
  MV = m_view * M;
  MVP = m_project * MV;
  normalMatrix = MV;
  normalMatrix.inverse().transpose();
  ngl::ShaderLib::setUniform("M", M);
  ngl::ShaderLib::setUniform("MV", MV);
  ngl::ShaderLib::setUniform("MVP", MVP);
  ngl::ShaderLib::setUniform("normalMatrix", normalMatrix);
  m_vao->bind();
  m_vao->draw();
  // now we are done so unbind
  m_vao->unbind();
}

//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quite
  case Qt::Key_Escape:
    QGuiApplication::exit(EXIT_SUCCESS);
    break;
  // turn on wirframe rendering
  case Qt::Key_W:
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    break;
  // turn off wire frame
  case Qt::Key_S:
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    break;
  // show full screen
  case Qt::Key_F:
    showFullScreen();
    break;
  // show windowed
  case Qt::Key_N:
    showNormal();
    break;
  default:
    break;
  }
  // finally update the GLWindow and re-draw
  // if (isExposed())
  update();
}
