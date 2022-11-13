#include "cascade_shadow_map_matrix.h"
#include "../../Camera/light_camera.h"
#include "../../Camera/Camera.h"

    //namespace shadow
    //{
    //    void CascadeShadowMapMatrix::CalcLightViewProjectionCropMatrix(DirectX::XMFLOAT3 lightDirection, camera main_camera,camera light_camera,float near_z,float far_z)
    //    {
    //        // ライトカメラを計算する
    //        DirectX::XMFLOAT4 lightPos;
    //        
    //        float distLig = main_camera.get_far_z() * 0.1f;
    //        lightPos = DirectX::XMFLOAT4{ lightDirection.x * -distLig,lightDirection.y * -distLig,lightDirection.z * -distLig ,0};    // ライトまでの距離は外から指定できるようにする
    //        light_camera.set_target(lightPos);
    //        light_camera.set_target(DirectX::XMFLOAT4{ 0.0f, 0.0f, 0.0f,0.0f });
    //        // 上方向を設定
    //        if (fabsf(lightDirection.y) > 0.9999f)
    //        {
    //            // ほぼ真上、真下を向いている
    //            light_camera.SetUp(g_vec3AxisX);
    //        }
    //        else
    //        {
    //            lightCamera.SetUp(g_vec3AxisY);
    //        }
    //        lightCamera.SetUpdateProjMatrixFunc(Camera::enUpdateProjMatrixFunc_Ortho);
    //        lightCamera.SetWidth(5000.0f);
    //        lightCamera.SetHeight(5000.0f);
    //        light_camera.active();
    //        const auto& lvpMatrix = lightCamera.GetViewProjectionMatrix();
    //        // カメラのファークリップから計算するようにする
    //        // 分割エリアの最大深度値を定義する
    //        float cascadeAreaTbl[NUM_SHADOW_MAP] = {
    //            main_camera.get_far_z() * 0.05f,    // 近影を映す最大深度値
    //           main_camera.get_far_z() * 0.3f,     // 中影を映す最大深度値
    //            main_camera.get_far_z(),            // 遠影を映す最大深度値。3枚目の最大深度はカメラのFarクリップ
    //        };
    //        // カメラの前方向、右方向、上方向を求める
    //        // 前方向と右方向はすでに計算済みなので、それを引っ張ってくる
    //        const auto& cameraForward = main_camera.get_front();
    //        const auto& cameraRight = main_camera.get_right();
    //        const auto& cameraUp = main_camera.get_up();
    //        float aspect_ratio = { 1280.0f / 720.0f };
    //        // nearDepthはエリアの最小深度値を表す
    //        // 一番近いエリアの最小深度値はカメラのニアクリップ
    //        float nearDepth = main_camera.get_near_z();
    //        for (int areaNo = 0; areaNo < NUM_SHADOW_MAP; areaNo++)
    //        {
    //            // エリアを内包する視錐台の8頂点を求める
    //            // エリアの近平面の中心からの上面、下面までの距離を求める
    //            float nearY = tanf(main_camera.get_view_angle() * 0.5f) * nearDepth;
    //            // エリアの近平面の中心からの右面、左面までの距離を求める
    //            float nearX = nearY * aspect_ratio;
    //            // エリアの遠平面の中心からの上面、下面までの距離を求める
    //            float farY = tanf(main_camera.get_view_angle() * 0.5f) * cascadeAreaTbl[areaNo];
    //            // エリアの遠平面の中心からの右面、左面までの距離を求める
    //            float farX = farY * aspect_ratio;
    //            // エリアの近平面の中心座標を求める
    //            DirectX::XMFLOAT4 nearPos = XMFLOAT4{ main_camera.position.x + cameraForward.x * nearDepth,main_camera.position.y + cameraForward.y * nearDepth,main_camera.position.z + cameraForward.z * nearDepth,main_camera.position.w + cameraForward.w * nearDepth };
    //            // エリアの遠平面の中心座標を求める
    //            XMFLOAT4 farPos = XMFLOAT4{ main_camera.position.x + cameraForward.x * cascadeAreaTbl[areaNo],main_camera.position.y + cameraForward.y * cascadeAreaTbl[areaNo] ,main_camera.position.z + cameraForward.z * cascadeAreaTbl[areaNo] ,main_camera.position.w + cameraForward.w * cascadeAreaTbl[areaNo] };
    //            // 8頂点を求める
    //            XMFLOAT4 vertex[8];
    //            // 近平面の右上の頂点
    //            vertex[0] = XMFLOAT4{ vertex[0].x + nearPos.x + cameraUp.x * nearY + cameraRight.x * nearX,vertex[0].y + nearPos.y + cameraUp.y * nearY + cameraRight.y * nearX,vertex[0].z +nearPos.z + cameraUp.z * nearY + cameraRight.z * nearX,vertex[0].w + nearPos.w + cameraUp.w * nearY + cameraRight.w * nearX };
    //            // 近平面の左上の頂点
    //            vertex[1] =  XMFLOAT4{ vertex[1].x + nearPos.x + cameraUp.x * nearY + cameraRight.x * -nearX,vertex[1].y + nearPos.y + cameraUp.y * nearY + cameraRight.y * -nearX,vertex[1].z + nearPos.z + cameraUp.z * nearY + cameraRight.z * -nearX,vertex[1].w + nearPos.w + cameraUp.w * nearY + cameraRight.w * -nearX };
    //            // 近平面の右下の頂点
    //            vertex[2] = XMFLOAT4{ vertex[2].x + nearPos.x + cameraUp.x * -nearY + cameraRight.x * nearX,
    //               vertex[2].y + nearPos.y + cameraUp.y * -nearY + cameraRight.y * nearX,
    //               vertex[2].z + nearPos.z + cameraUp.z * -nearY + cameraRight.z * nearX,
    //               vertex[2].w + nearPos.w + cameraUp.w * -nearY + cameraRight.w * nearX };
    //            // 近平面の左下の頂点
    //            vertex[3] = XMFLOAT4{ vertex[3].x + nearPos.x + cameraUp.x * -nearY + cameraRight.x * -nearX,
    //                vertex[3].y + nearPos.y + cameraUp.y * -nearY + cameraRight.y * -nearX,
    //                vertex[3].z + nearPos.z + cameraUp.z * -nearY + cameraRight.z * -nearX,
    //                vertex[3].w + nearPos.w + cameraUp.w * -nearY + cameraRight.w * -nearX };
    //            // 遠平面の右上の頂点
    //            vertex[4] = XMFLOAT4{
    //                vertex[4].x + farPos.x + cameraUp.x * farY + cameraRight.x * farX,
    //                vertex[4].y + farPos.y + cameraUp.y * farY + cameraRight.y * farX,
    //                vertex[4].z + farPos.z + cameraUp.z * farY + cameraRight.z * farX,
    //                vertex[4].w + farPos.w + cameraUp.w * farY + cameraRight.w * farX };
    //            // 遠平面の左上の頂点
    //               vertex[5] = XMFLOAT4{
    //          vertex[5].x + farPos.x + cameraUp.x * farY + cameraRight.x *-farX,
    //          vertex[5].y + farPos.y + cameraUp.y * farY + cameraRight.y *-farX,
    //          vertex[5].z + farPos.z + cameraUp.z * farY + cameraRight.z *-farX,
    //          vertex[5].w + farPos.w + cameraUp.w * farY + cameraRight.w * -farX };
    //            // 遠平面の右下の頂点
    //            vertex[6] = XMFLOAT4{
    //          vertex[6].x + farPos.x + cameraUp.x * -farY + cameraRight.x * farX,
    //          vertex[6].y + farPos.y + cameraUp.y * -farY + cameraRight.y * farX,
    //          vertex[6].z + farPos.z + cameraUp.z * -farY + cameraRight.z * farX,
    //          vertex[6].w + farPos.w + cameraUp.w * -farY + cameraRight.w * farX };
    //            // 遠平面の左下の頂点
    //            vertex[7] = XMFLOAT4{
    //            vertex[7].x + farPos.x + cameraUp.x * -farY + cameraRight.x * -farX,
    //            vertex[7].y + farPos.y + cameraUp.y * -farY + cameraRight.y * -farX,
    //            vertex[7].z + farPos.z + cameraUp.z * -farY + cameraRight.z * -farX,
    //            vertex[7].w + farPos.w + cameraUp.w * -farY + cameraRight.w * -farX };
    //            // 8頂点をライトビュープロジェクション空間に変換して、8頂点の最大値、最小値を求める
    //            XMFLOAT3 vMax, vMin;
    //            vMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
    //            vMin = { FLT_MAX,  FLT_MAX,  FLT_MAX };
    //            for (auto& v : vertex)
    //            {
    //                DirectX::XMFLOAT3 xyz = DirectX::XMFLOAT3{ v.x,v.y,v.z };
    //                XMVECTOR v_vec = DirectX::XMLoadFloat4(&v);
    //                DirectX::XMStoreFloat3(
    //                    &xyz,
    //                    DirectX::XMVector3Transform(v_vec, *lvpMatrix)
    //                );
    //                vMax.x = max(vMax.x, v.x);
    //                vMax.y = max(vMax.y, v.y );
    //                vMax.z = max(vMax.z, v.z );
    //                vMin.x = min(vMin.x, v.x);
    //                vMin.y = min(vMin.y, v.y);
    //                vMin.z = min(vMin.z, v.z);
    //            }
    //            // クロップ行列を求める
    //            float xScale = 2.0f / (vMax.x - vMin.x);
    //            float yScale = 2.0f / (vMax.y - vMin.y);
    //            float xOffset = (vMax.x + vMin.x) * -0.5f * xScale;
    //            float yOffset = (vMax.y + vMin.y) * -0.5f * yScale;
    //            DirectX::XMFLOAT4X4 clopMatrix;
    //            clopMatrix.m[0][0] = xScale;
    //            clopMatrix.m[1][1] = yScale;
    //            clopMatrix.m[3][0] = xOffset;
    //            clopMatrix.m[3][1] = yOffset;
    //            // ライトビュープロジェクション行列にクロップ行列を乗算する
    //            m_lvpcMatrix[areaNo] = lvpMatrix * clopMatrix;
    //            // 次のエリアの近平面までの距離を代入する
    //            nearDepth = cascadeAreaTbl[areaNo];
    //        }
    //    }
    //}


    namespace shadow
    {
        void CascadeShadowMapMatrix::CalcLightViewProjectionCropMatrix(Vector3 lightDirection, float cam_far,float cam_near,XMFLOAT4 cam_front,XMFLOAT4 cam_right,XMFLOAT4 cam_position)
        {
            // ライトカメラを計算する
            light_camera lightCamera;
            Vector3 lightPos;
            float distLig = cam_far * 0.1f;
            lightPos = lightDirection * -distLig;    // ライトまでの距離は外から指定できるようにする
            lightPos.vec = XMFLOAT3{ cam_position.x,cam_position.y,cam_position.z };
            lightCamera.SetPosition(lightPos);
            lightCamera.SetTarget(0.0f, 0.0f, 0.0f);
            // 上方向を設定
            if (fabsf(lightDirection.y) > 0.9999f)
            {
                // ほぼ真上、真下を向いている
                lightCamera.SetUp(g_vec3AxisX);
            }
            else
            {
                lightCamera.SetUp(g_vec3AxisY);
            }
            lightCamera.SetUpdateProjMatrixFunc(light_camera::enUpdateProjMatrixFunc_Ortho);
            lightCamera.SetWidth(5000.0f);
            lightCamera.SetHeight(5000.0f);
            lightCamera.SetNear(1.0f);
            lightCamera.SetFar(cam_far);
            lightCamera.Update();

            const auto& lvpMatrix = lightCamera.GetViewProjectionMatrix();

            // カメラのファークリップから計算するようにする
            // 分割エリアの最大深度値を定義する
            float cascadeAreaTbl[NUM_SHADOW_MAP] = {
                 cam_far * 0.05f,    // 近影を映す最大深度値
                 cam_far * 0.3f,     // 中影を映す最大深度値
                 cam_far,            // 遠影を映す最大深度値。3枚目の最大深度はカメラのFarクリップ
            };

            // カメラの前方向、右方向、上方向を求める
            // 前方向と右方向はすでに計算済みなので、それを引っ張ってくる
            Vector3 camera_front;
            Vector3 camera_right;
            camera_front.vec = XMFLOAT3{ cam_front.x, cam_front.y , cam_front.z };
            camera_right.vec = XMFLOAT3{ cam_right.x, cam_right.y , cam_right.z };
            const auto& cameraForward = camera_front;
            const auto& cameraRight = camera_right;

            DirectX::XMFLOAT3 forward = XMFLOAT3{ cameraForward.x,cameraForward.y,cameraForward.z };
            DirectX::XMFLOAT3 right = XMFLOAT3{ cameraRight.x,cameraRight.y,cameraRight.z };
            // カメラの上方向は前方向と右方向の外積で求める
            Vector3 cameraUp;
            DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&forward);
            DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&right);
            DirectX::XMVECTOR xmvr = DirectX::XMVector3Cross(xmv0, xmv1);
            DirectX::XMStoreFloat3(&cameraUp.vec, xmvr);

            // nearDepthはエリアの最小深度値を表す
            // 一番近いエリアの最小深度値はカメラのニアクリップ
            float nearDepth = cam_near;
            for (int areaNo = 0; areaNo < NUM_SHADOW_MAP; areaNo++)
            {
                // エリアを内包する視錐台の8頂点を求める
                // エリアの近平面の中心からの上面、下面までの距離を求める
                float nearY = tanf(lightCamera.GetViewAngle() * 0.5f) * nearDepth;

                // エリアの近平面の中心からの右面、左面までの距離を求める
                float nearX = nearY * lightCamera.GetAspect();

                // エリアの遠平面の中心からの上面、下面までの距離を求める
                float farY = tanf(lightCamera.GetViewAngle() * 0.5f) * cascadeAreaTbl[areaNo];

                // エリアの遠平面の中心からの右面、左面までの距離を求める
                float farX = farY * lightCamera.GetAspect();

                Vector3 camera_pos;
                camera_pos.vec = XMFLOAT3{ cam_position.x,cam_position.y, cam_position.z };
                // エリアの近平面の中心座標を求める
                Vector3 nearPos = camera_pos + cameraForward * nearDepth;

                // エリアの遠平面の中心座標を求める
                Vector3 farPos = camera_pos + cameraForward * cascadeAreaTbl[areaNo];

                // 8頂点を求める
                Vector3 vertex[8];

                // 近平面の右上の頂点
                vertex[0] += nearPos + cameraUp * nearY + cameraRight * nearX;

                // 近平面の左上の頂点
                vertex[1] += nearPos + cameraUp * nearY + cameraRight * -nearX;

                // 近平面の右下の頂点
                vertex[2] += nearPos + cameraUp * -nearY + cameraRight * nearX;

                // 近平面の左下の頂点
                vertex[3] += nearPos + cameraUp * -nearY + cameraRight * -nearX;

                // 遠平面の右上の頂点
                vertex[4] += farPos + cameraUp * farY + cameraRight * farX;

                // 遠平面の左上の頂点
                vertex[5] += farPos + cameraUp * farY + cameraRight * -farX;

                // 遠平面の右下の頂点
                vertex[6] += farPos + cameraUp * -farY + cameraRight * farX;

                // 遠平面の左下の頂点
                vertex[7] += farPos + cameraUp * -farY + cameraRight * -farX;

                // 8頂点をライトビュープロジェクション空間に変換して、8頂点の最大値、最小値を求める
                Vector3 vMax, vMin;
                vMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
                vMin = { FLT_MAX,  FLT_MAX,  FLT_MAX };
                for (auto& v : vertex)
                {
                    lvpMatrix.Apply(v);
                    vMax.Max(v);
                    vMin.Min(v);
                }

                // クロップ行列を求める
                float xScale = 2.0f / (vMax.x - vMin.x);
                float yScale = 2.0f / (vMax.y - vMin.y);
                float xOffset = (vMax.x + vMin.x) * -0.5f * xScale;
                float yOffset = (vMax.y + vMin.y) * -0.5f * yScale;
                Matrix clopMatrix;
                clopMatrix.m[0][0] = xScale;
                clopMatrix.m[1][1] = yScale;
                clopMatrix.m[3][0] = xOffset;
                clopMatrix.m[3][1] = yOffset;

                // ライトビュープロジェクション行列にクロップ行列を乗算する
                m_lvpcMatrix[areaNo] = lvpMatrix * clopMatrix;

                // 次のエリアの近平面までの距離を代入する
                nearDepth = cascadeAreaTbl[areaNo];
            }
        }
    }
