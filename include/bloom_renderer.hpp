#ifndef BLOOM_RENDERER_H
#define BLOOM_RENDERER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "shader.hpp"

class BloomRenderer {
public:
    BloomRenderer(unsigned int width, unsigned int height);
    ~BloomRenderer();
    
    void resize(unsigned int width, unsigned int height);
    void beginScene();
    void renderBloom(unsigned int srcTexture);
    void renderToScreen();
    
    unsigned int getSceneTexture() const { return mHdrColorBuffer; }
    unsigned int getBrightTexture() const { return mHdrBrightBuffer; }
    
private:
    void init();
    void cleanup();
    void createFramebuffers();
    void createScreenQuad();
    
    unsigned int mWidth, mHeight;
    
    // HDR framebuffer (scene rendering)
    unsigned int mHdrFBO;
    unsigned int mHdrColorBuffer;
    unsigned int mHdrBrightBuffer;
    unsigned int mHdrDepthBuffer;
    
    // Bloom mip chain
    struct BloomMip {
        glm::vec2 size;
        unsigned int texture;
        unsigned int fbo;
    };
    std::vector<BloomMip> mMipChain;
    
    // Screen quad
    unsigned int mQuadVAO, mQuadVBO;
    
    // Shaders
    Shader* mDownsampleShader;
    Shader* mUpsampleShader;
    Shader* mCompositeShader;
    
    // Bloom parameters
    float mFilterRadius;
    float mBloomStrength;
    
    static const unsigned int NUM_MIPS = 6;
};

#endif