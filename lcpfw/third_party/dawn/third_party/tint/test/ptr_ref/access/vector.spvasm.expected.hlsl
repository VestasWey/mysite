[numthreads(1, 1, 1)]
void main() {
  float3 v = float3(0.0f, 0.0f, 0.0f);
  v = float3(1.0f, 2.0f, 3.0f);
  v.y = 5.0f;
  return;
}
