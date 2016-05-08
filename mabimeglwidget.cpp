#include "mabimeglwidget.h"
#include <QtWidgets>
#include <QtOpenGL>
#include <QDebug>
#include "GL/gl.h"
#include "GL/glu.h"

void MabiMeGLWidget::checkError(QString error) {
    QString finalError = "";
    GLenum a = glGetError();
    if (a == GL_INVALID_ENUM) finalError = "GL_INVALID_ENUM";
    if (a == GL_INVALID_VALUE) finalError = "GL_INVALID_VALUE";
    if (a == GL_INVALID_OPERATION) finalError = "GL_INVALID_OPERATION";
    if (a == GL_INVALID_FRAMEBUFFER_OPERATION) finalError = "GL_INVALID_FRAMEBUFFER_OPERATION";
    if (a == GL_OUT_OF_MEMORY) finalError = "GL_OUT_OF_MEMORY";
    if (a == GL_STACK_UNDERFLOW) finalError = "GL_STACK_UNDERFLOW";
    if (a == GL_STACK_OVERFLOW) finalError = "GL_STACK_OVERFLOW";
    if (finalError.length() > 0) {
        finalError = error + " - " + finalError;
        qDebug() << finalError;
    }

}

#if defined(Q_OS_WIN)
    void *GetAnyGLFuncAddress(const char *name)
    {
        void *p = (void *)wglGetProcAddress(name);
        if(p == 0 ||
            (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) ||
            (p == (void*)-1) )
        {
            HMODULE module = LoadLibraryA("opengl32.dll");
            p = (void *)GetProcAddress(module, name);
        }
        return p;
    }
#elif defined (Q_OS_LINUX)
    void *GetAnyGLFuncAddress(const char *name)
    {
        void *p = (void *)glXGetProcAddress(name);
        return p;
    }
#endif

MabiMeGLWidget::MabiMeGLWidget(QWidget *parent) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
}

void MabiMeGLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(camera.x, camera.y, camera.zoom);
    for (int i = 0; i < objects.count(); i++) {
        PMGObject *o = objects[i];
        for (int n = 0; n < o->meshes.count(); n++) {
            PMGTexture *t = nullptr;
            FRM::Bone *b = nullptr;
            for (int i = 0; i < o->textures.count(); i++) {
                if (o->textures[i]->name == o->meshes[n]->textureName) {
                    t = o->textures[i];
                    break;
                }
            }
            for (int i = 0; i < o->bones.count(); i++) {
                if (o->bones[i]->name == o->meshes[n]->boneName) {
                    b = o->bones[i];
                    break;
                }
            }
            QList<FRM::Bone*> bl = QList<FRM::Bone*>();
            if (b != nullptr) {
                while (true) {
                    bl.insert(0, b);
                    char parent = b->parentID;
                    b = nullptr;
                    for (int i = 0; i < o->bones.count(); i++) {
                        if (o->bones[i]->boneID == parent) {
                            b = o->bones[i];
                            break;
                        }
                    }
                    if (b == nullptr) break;
                }
            }
            if (bl.count() > 0) {
                renderPMGMesh(*o->meshes[n], &bl, t);
            } else {
                renderPMGMesh(*o->meshes[n], nullptr, t);
            }
        }
    }
}

void MabiMeGLWidget::initializeGL() {
    // load extensions
    glEnableVertexAttribArray   = (PFNGLENABLEVERTEXATTRIBARRAYPROC)GetAnyGLFuncAddress("glEnableVertexAttribArray");
    glVertexAttribPointer       = (PFNGLVERTEXATTRIBPOINTERPROC)GetAnyGLFuncAddress("glVertexAttribPointer");
    glGetAttribLocation         = (PFNGLGETATTRIBLOCATIONPROC)GetAnyGLFuncAddress("glGetAttribLocation");
    glGetShaderInfoLog          = (PFNGLGETSHADERINFOLOGPROC)GetAnyGLFuncAddress("glGetShaderInfoLog");
    glGetProgramInfoLog         = (PFNGLGETPROGRAMINFOLOGPROC)GetAnyGLFuncAddress("glGetProgramInfoLog");
    glCreateProgram             = (PFNGLCREATEPROGRAMPROC)GetAnyGLFuncAddress("glCreateProgram");
    glCreateShader              = (PFNGLCREATESHADERPROC)GetAnyGLFuncAddress("glCreateShader");
    glShaderSource              = (PFNGLSHADERSOURCEPROC)GetAnyGLFuncAddress("glShaderSource");
    glCompileShader             = (PFNGLCOMPILESHADERPROC)GetAnyGLFuncAddress("glCompileShader");
    glAttachShader              = (PFNGLATTACHSHADERPROC)GetAnyGLFuncAddress("glAttachObjectARB");
    glLinkProgram               = (PFNGLLINKPROGRAMPROC)GetAnyGLFuncAddress("glLinkProgram");
    glUseProgram                = (PFNGLUSEPROGRAMPROC)GetAnyGLFuncAddress("glUseProgramObjectARB");
    glGetUniformLocation        = (PFNGLGETUNIFORMLOCATIONPROC)GetAnyGLFuncAddress("glGetUniformLocation");
    glUniform1i                 = (PFNGLUNIFORM1IPROC)GetAnyGLFuncAddress("glUniform1i");
    glUniform1iv                = (PFNGLUNIFORM1IVPROC)GetAnyGLFuncAddress("glUniform1iv");
    glUniform1f                 = (PFNGLUNIFORM1FPROC)GetAnyGLFuncAddress("glUniform1f");
    glUniform2f                 = (PFNGLUNIFORM2FPROC)GetAnyGLFuncAddress("glUniform2f");
    glUniform3f                 = (PFNGLUNIFORM3FPROC)GetAnyGLFuncAddress("glUniform3f");
    glUniform4f                 = (PFNGLUNIFORM4FPROC)GetAnyGLFuncAddress("glUniform4f");
    glUniform4fv                = (PFNGLUNIFORM4FVPROC)GetAnyGLFuncAddress("glUniform4fv");
    glBindAttribLocation        = (PFNGLBINDATTRIBLOCATIONPROC)GetAnyGLFuncAddress("glBindAttribLocation");
    glGenFramebuffers           = (PFNGLGENFRAMEBUFFERSEXTPROC)GetAnyGLFuncAddress("glGenFramebuffersEXT");
    glBindFramebuffer           = (PFNGLBINDFRAMEBUFFEREXTPROC)GetAnyGLFuncAddress("glBindFramebufferEXT");
    glFramebufferTexture2D      = (PFNGLFRAMEBUFFERTEXTURE2DPROC)GetAnyGLFuncAddress("glFramebufferTexture2D");
    glBlendFuncSeparate         = (PFNGLBLENDFUNCSEPARATEPROC)GetAnyGLFuncAddress("glBlendFuncSeparate");
    glBindBuffer                = (PFNGLBINDBUFFERPROC)GetAnyGLFuncAddress("glBindBuffer");
    glBufferData                = (PFNGLBUFFERDATAPROC)GetAnyGLFuncAddress("glBufferData");
    glGenBuffers                = (PFNGLGENBUFFERSPROC)GetAnyGLFuncAddress("glGenBuffers");
    glActiveTexture             = (PFNGLACTIVETEXTUREPROC)GetAnyGLFuncAddress("glActiveTexture");
    glClientActiveTexture       = (PFNGLCLIENTACTIVETEXTUREPROC)GetAnyGLFuncAddress("glClientActiveTexture");

    qglClearColor(QColor(200, 200, 200));
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_TEXTURE_2D);
    glEnable (GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GEQUAL, 1);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);


    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    static GLfloat lightPosition[4] = { 0, 0, 10, 1.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    emit cameraChange(camera);
}

void Perspective( GLdouble fov, GLdouble aspect, GLdouble zNear, GLdouble zFar )
{
    GLdouble m[16] = { 1,0,0,0,0,1,0,0,0,0,1,-1,0,0,0,0 };
    double sine, cotangent, deltaZ;
    double radians = fov / 2 * 3.14159265358979323846 / 180;

    deltaZ = zFar - zNear;
    sine = sin(radians);
    if ((deltaZ == 0) || (sine == 0) || (aspect == 0)) {
    return;
    }
    cotangent = cos(radians) / sine;

    m[0] = cotangent / aspect;
    m[5] = cotangent;
    m[10] = -(zFar + zNear) / deltaZ;
    m[14] = -2 * zNear * zFar / deltaZ;
    glMultMatrixd(&m[0]);
}

void MabiMeGLWidget::resizeGL(int width, int height) {
    int side = qMin(width, height);
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    Perspective(60, (GLfloat)width / (GLfloat)height, 1.0, 4000.0);
    glMatrixMode(GL_MODELVIEW);
}

void MabiMeGLWidget::renderPMGMesh(PMG::Mesh mesh, QList<FRM::Bone *> *bones, PMGTexture *t) {
    qglColor(Qt::red);
    glPushMatrix();
    glRotatef(camera.rotation.pitch, 1.0, 0.0, 0.0);
    glRotatef(camera.rotation.yaw, 0.0, 1.0, 0.0);
    if (bones != nullptr) { // if bones, multiply the bone parents together
        for (int i = 0; i < bones->count(); i++) {
            glMultMatrixf(bones->at(i)->link);
        }
        glMultMatrixf(mesh.minorMatrix.data());
    } else {
        glMultMatrixf(mesh.majorMatrix.data());
    }
    glVertexPointer(3, GL_FLOAT, 0, mesh.cleanVertices);
    glColorPointer(4, GL_FLOAT, 0, mesh.cleanColours);
    glNormalPointer(GL_FLOAT, 0, mesh.cleanNormals);
    glTexCoordPointer(2, GL_FLOAT, 0, mesh.cleanTextureCoords);
    glActiveTexture(GL_TEXTURE0);
    if (t != nullptr) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glBindTexture(GL_TEXTURE_2D, t->texture);
    } else {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    checkError("glBindTexture");
    glDrawArrays(GL_TRIANGLES, 0, mesh.cleanVertexCount);
    glPopMatrix();
}

void MabiMeGLWidget::mousePressEvent(QMouseEvent *event) {
    if (isLeftDragging || isRightDragging) return;
    if (event->buttons() == Qt::LeftButton) {
        isLeftDragging = true;
        drag = QPoint(event->x(), event->y());
        oldCameraPos = QPoint(camera.x, camera.y);
        return;
    } else {
        isLeftDragging = false;
    }
    if (event->buttons() == Qt::RightButton) {
        isRightDragging = true;
        drag = QPoint(event->x(), event->y());
        oldCameraRotation = camera.rotation;
        return;
    } else {
        isLeftDragging = false;
    }
}
void MabiMeGLWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->buttons() != Qt::LeftButton) isLeftDragging = false;
    if (event->buttons() != Qt::RightButton) isRightDragging = false;
}
void MabiMeGLWidget::mouseMoveEvent(QMouseEvent *event) {
    if (isLeftDragging) {
        camera.x = oldCameraPos.x() + ((event->x() - drag.x()) / 4);
        camera.y = oldCameraPos.y() - ((event->y() - drag.y()) / 4);
    }
    if (isRightDragging) {
        camera.rotation.pitch = oldCameraRotation.pitch + (event->y() - drag.y());
        camera.rotation.yaw = oldCameraRotation.yaw + (event->x() - drag.x());
        camera.rotation.pitch = camera.rotation.pitch % 360;
        if (camera.rotation.pitch < 0) camera.rotation.pitch += 360;
        camera.rotation.yaw = camera.rotation.yaw % 360;
        if (camera.rotation.yaw < 0) camera.rotation.yaw += 360;
    }
    if (event->x() != drag.x() || event->y() != drag.y()) {
        this->repaint();
        emit cameraChange(camera);
    }
}

void MabiMeGLWidget::wheelEvent(QWheelEvent *event) {
    if (event->angleDelta().y() > 0) {
        camera.zoom += 10;
        this->repaint();
        emit cameraChange(camera);
    } else if(event->angleDelta().y() < 0) {
        camera.zoom -= 10;
        this->repaint();
        emit cameraChange(camera);
    }
}

bool MabiMeGLWidget::loadTexture(PMGTexture *t, bool useFiltering) {
    glGenTextures(1, &t->texture);
    glBindTexture(GL_TEXTURE_2D, t->texture);
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if (!useFiltering) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, 4, t->img.width(), t->img.height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, t->img.bits());
    return true;
}

bool MabiMeGLWidget::addPMG(PMGObject *pmg) {
    for (int i = 0; i < pmg->textures.count(); i++) {
        loadTexture(pmg->textures[i], true);
    }
    objects.append(pmg);
}
