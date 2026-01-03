module camera;

public struct Camera {
    public float4x4 view;
    public float4x4 projection;
    public float4x4 inverseView;
    public float4x4 inverseProjection;

    public float4[6] GetFrustumPlanes() {
        float4 planes[6];

        float4x4 vp = mul(projection, view);

        planes[0] = vp[3] + vp[0];
        planes[1] = vp[3] - vp[0];
        planes[2] = vp[3] + vp[1];
        planes[3] = vp[3] - vp[1];
        planes[4] = vp[3] + vp[2];
        planes[5] = vp[3] - vp[2];

        for (int i = 0; i < 6; i++) {
            float length = length(planes[i].xyz);
            planes[i] /= length;
        }

        return planes;
    }

    public func IsBoxInFrustum(float3 boxMin, float3 boxMax) -> bool {
        float4 planes[6] = GetFrustumPlanes();

        for (int i = 0; i < 6; i++) {
            float3 planeNormal = planes[i].xyz;
            float planeDistance = planes[i].w;

            float3 positiveVertex = boxMin;
            if (planeNormal.x >= 0) positiveVertex.x = boxMax.x;
            if (planeNormal.y >= 0) positiveVertex.y = boxMax.y;
            if (planeNormal.z >= 0) positiveVertex.z = boxMax.z;

            if (dot(planeNormal, positiveVertex) + planeDistance < 0) {
                return false;
            }
        }
        return true;
    }
};