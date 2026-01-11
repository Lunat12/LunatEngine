#include "Globals.h"
#include "ModuleEditorCamera.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "ModuleD3D12.h"
#include "Application.h"

namespace
{
    constexpr float GetRotationSpeed() { return 25.0f; }
    constexpr float GetTranslationSpeed() { return 2.5f; }
}

bool ModuleEditorCamera::init()
{
    /*position = Vector3(0, 1, 10);

    Matrix myRotation = Matrix::CreateFromYawPitchRoll(Vector3::Zero);

    myRotation.Transpose();

    Vector4 translation;
    Vector4::Transform(Vector4(-position.x, -position.y, -position.z, 1), myRotation, translation);
    view.Translation(Vector3(translation.x, translation.y, translation.z));*/

    ModuleD3D12* d3d12 = app->getD3D12();
    aspect = float(d3d12->getWindowWidth()) / float(d3d12->getWindowHeight());

    position = Vector3(0.0f, 1.0f, 10.0f);
    rotation = Quaternion::CreateFromAxisAngle(Vector3(0.0f, 1.0f, 0.0f), XMConvertToRadians(0.0f));

    Quaternion invRot;
    rotation.Inverse(invRot);

    view = Matrix::CreateFromQuaternion(invRot);

    view.Translation(-position);

    return true;
}

void ModuleEditorCamera::update()
{
   /* ModuleD3D12* d3d12 = app->getD3D12();
    aspect = float(d3d12->getWindowWidth()) / float(d3d12->getWindowHeight());

    Vector3 translate = Vector3::Zero;
    Vector2 rotate = Vector2::Zero;

    Mouse& mouse = Mouse::Get();
    const Mouse::State& mouseState = mouse.GetState();
    
    Keyboard& keyboard = Keyboard::Get();
    const Keyboard::State& keyboardState = keyboard.GetState();

    if (mouseState.rightButton)
    {
        rotate.y = float(DragPosX - mouseState.x) * 0.005f;
        rotate.x = float(DragPosY - mouseState.y) * 0.005f;
    }

    if (keyboardState.W) translate.z -= 0.45f;
    if (keyboardState.A) translate.x -= 0.45f;
    if (keyboardState.S) translate.z += 0.45f;
    if (keyboardState.D) translate.x += 0.45f;
    if (keyboardState.Q) translate.y += 0.45f;
    if (keyboardState.E) translate.y -= 0.45f;

    rotation.x += rotate.x;
    rotation.y += rotate.y;

    view = Matrix::CreateFromYawPitchRoll(rotation);
    view.Transpose();
    view.Translation(Vector3::Transform(-position, view));

    DragPosX = mouseState.x;
    DragPosY = mouseState.y;*/

    Mouse& mouse = Mouse::Get();
    const Mouse::State& mouseState = mouse.GetState();

    Keyboard& keyboard = Keyboard::Get();
    const Keyboard::State& keyState = keyboard.GetState();

    float elapsedSec = app->getElapsedMilis() * 0.001f;

    Vector3 translate = Vector3::Zero;
    Vector2 rotate = Vector2::Zero;

    if (mouseState.rightButton)
    {
        rotate.x = float(DragPosX - mouseState.x) * 0.005f;
        rotate.y = float(DragPosY - mouseState.y) * 0.005f;

        if (keyState.W) translate.z -= 0.45f * elapsedSec;
        if (keyState.A) translate.x -= 0.45f * elapsedSec;
        if (keyState.S) translate.z += 0.45f * elapsedSec;
        if (keyState.D) translate.x += 0.45f * elapsedSec;
        if (keyState.Q) translate.y += 0.45f * elapsedSec;
        if (keyState.E) translate.y -= 0.45f * elapsedSec;
    }

    if (mouseState.scrollWheelValue > 0) 
    {
        fov -= 0.1f;
        mouse.ResetScrollWheelValue();
    }
    else if (mouseState.scrollWheelValue < 0) 
    {
        fov += 0.1f;
        mouse.ResetScrollWheelValue();
    }


    if (keyState.F) 
    {
        view = Matrix::CreateLookAt(position, Vector3::Zero, Vector3::Up);
        return;
    }

    float movementSpeed = GetTranslationSpeed();
    if (keyState.LeftShift)
    {
        movementSpeed = movementSpeed * 2;
    }

    Vector3 localDir = Vector3::Transform(translate, rotation);
    params.panning += localDir * movementSpeed;
    params.polar += XMConvertToRadians(GetRotationSpeed() * rotate.x);
    params.azimuthal += XMConvertToRadians(GetRotationSpeed() * rotate.y);

    Quaternion rotation_polar = Quaternion::CreateFromAxisAngle(Vector3(0.0f, 1.0f, 0.0f), params.polar);
    Quaternion rotation_azimuthal = Quaternion::CreateFromAxisAngle(Vector3(1.0f, 0.0f, 0.0f), params.azimuthal);
    rotation = rotation_azimuthal * rotation_polar;
    position = params.panning;

    Quaternion invRot;
    rotation.Inverse(invRot);

    view = Matrix::CreateFromQuaternion(invRot);

    if (keyState.LeftAlt)
    {
        view.Translation(Vector3::Transform(-position, Matrix::CreateLookAt(position, Vector3::Zero, Vector3::Up)));
    }
    else 
    {
        view.Translation(Vector3::Transform(-position, invRot));
    }


    DragPosX = mouseState.x;
    DragPosY = mouseState.y;
}

void ModuleEditorCamera::setFOV(float _fov)
{
    fov = _fov;
}

void ModuleEditorCamera::setAspectRatio(float _aspect)
{
    aspect = _aspect;
}

void ModuleEditorCamera::setPlaneDistance(float _nearPlane, float _farPlane)
{
    nearPlane = _nearPlane;
    farPlane = _farPlane;
}

const Matrix ModuleEditorCamera::GetProjectionMatrix(float aspect)
{
    return Matrix::CreatePerspectiveFieldOfView(fov, aspect, nearPlane, farPlane);
}
