#pragma once
#include <array>
#include "../../Math/matrix.h"
#define NUM_SHADOW_MAP 3
#define NUM_DEFERRED_LIGHTING_DIRECTIONAL_LIGHT 1

    namespace shadow
    {
        /// <summary>
        /// カスケードシャドウマップ法で使用される行列クラス
        /// </summary>
        class CascadeShadowMapMatrix
        {
        public:
            /// <summary>
            /// ライトビュープロジェクションクロップ行列を計算する
            /// </summary>
            /// <param name="lightDirection">ライトの方向</param>
            //void CalcLightViewProjectionCropMatrix(DirectX::XMFLOAT3 lightDirection, camera main_camera, camera light_camera, float near_z, float far_z);
            void CalcLightViewProjectionCropMatrix(Vector3 lightDirection, float cam_far, float cam_near, DirectX::XMFLOAT4 cam_front, DirectX::XMFLOAT4 cam_right, DirectX::XMFLOAT4 cam_position);

            /// <summary>
            /// 計算されたライトビュープロジェクションクロップ行列を取得
            /// </summary>
            /// <param name="shadowMapNo">シャドウマップの番号</param>
            /// <returns></returns>
            const Matrix& GetLightViewProjectionCropMatrix(int shadowMapNo) const
            {
                return m_lvpcMatrix[shadowMapNo];
            }

        private:
            std::array<Matrix, NUM_SHADOW_MAP> m_lvpcMatrix;		//ライトビュークロップ行列
        };
    }
