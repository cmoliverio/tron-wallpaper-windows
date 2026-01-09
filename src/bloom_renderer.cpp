#include "bloom_renderer.hpp"
#include <iostream>
#include <glm/glm.hpp>

BloomRenderer::BloomRenderer(unsigned int width, unsigned int height)
    : mWidth(width), mHeight(height), mFilterRadius(0.005f), mBloomStrength(0.04f) {
    init();
}

BloomRenderer::~BloomRenderer() {
    cleanup();
}

void BloomRenderer::init() {
    createFramebuffers();
    createScreenQuad();
    
    // Load shaders
    mDownsampleShader = new Shader("downsample.vert", "downsample.frag");
    mUpsampleShader = new Shader("upsample.vert", "upsample.frag");
    mCompositeShader = new Shader("composite.vert", "composite.frag");
}

void BloomRenderer::cleanup() {
    // Delete HDR framebuffer
    glDeleteFramebuffers(1, &mHdrFBO);
    glDeleteTextures(1, &mHdrColorBuffer);
    glDeleteTextures(1, &mHdrBrightBuffer);
    glDeleteRenderbuffers(1, &mHdrDepthBuffer);
    
    // Delete mip chain
    for (auto& mip : mMipChain) {
        glDeleteFramebuffers(1, &mip.fbo);
        glDeleteTextures(1, &mip.texture);
    }
    mMipChain.clear();
    
    // Delete screen quad
    glDeleteVertexArrays(1, &mQuadVAO);
    glDeleteBuffers(1, &mQuadVBO);
    
    // Delete shaders
    delete mDownsampleShader;
    delete mUpsampleShader;
    delete mCompositeShader;
}

void BloomRenderer::createFramebuffers() {
    // Create HDR framebuffer with 2 color attachments
    glGenFramebuffers(1, &mHdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, mHdrFBO);
    
    // Color buffer (scene)
    glGenTextures(1, &mHdrColorBuffer);
    glBindTexture(GL_TEXTURE_2D, mHdrColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, mWidth, mHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mHdrColorBuffer, 0);
    
    // Bright buffer (bright parts extraction)
    glGenTextures(1, &mHdrBrightBuffer);
    glBindTexture(GL_TEXTURE_2D, mHdrBrightBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, mWidth, mHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mHdrBrightBuffer, 0);
    
    // Depth buffer
    glGenRenderbuffers(1, &mHdrDepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, mHdrDepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, mWidth, mHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mHdrDepthBuffer);
    
    // Tell OpenGL which color attachments we'll use
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "HDR Framebuffer not complete!" << std::endl;
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // Create bloom mip chain
    glm::vec2 mipSize((float)mWidth, (float)mHeight);
    for (unsigned int i = 0; i < NUM_MIPS; i++) {
        BloomMip mip;
        
        mipSize *= 0.5f;
        mip.size = mipSize;
        
        glGenTextures(1, &mip.texture);
        glBindTexture(GL_TEXTURE_2D, mip.texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, (int)mipSize.x, (int)mipSize.y, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        glGenFramebuffers(1, &mip.fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, mip.fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mip.texture, 0);
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Bloom mip framebuffer " << i << " not complete!" << std::endl;
        
        mMipChain.push_back(mip);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void BloomRenderer::createScreenQuad() {
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    
    glGenVertexArrays(1, &mQuadVAO);
    glGenBuffers(1, &mQuadVBO);
    glBindVertexArray(mQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
}

void BloomRenderer::beginScene() {
    glBindFramebuffer(GL_FRAMEBUFFER, mHdrFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void BloomRenderer::renderBloom(unsigned int srcTexture) {
    // Downsample
    mDownsampleShader->use();
    mDownsampleShader->setInt("srcTexture", 0);
    
    // Bind source texture (bright buffer)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTexture);
    
    for (unsigned int i = 0; i < mMipChain.size(); i++) {
        const BloomMip& mip = mMipChain[i];
        
        glViewport(0, 0, (int)mip.size.x, (int)mip.size.y);
        glBindFramebuffer(GL_FRAMEBUFFER, mip.fbo);
        
        mDownsampleShader->setVec2("srcResolution", 
            i == 0 ? glm::vec2(mWidth, mHeight) : mMipChain[i - 1].size);
        
        glBindVertexArray(mQuadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // Set current mip as source for next iteration
        glBindTexture(GL_TEXTURE_2D, mip.texture);
    }
    
    // Upsample
    mUpsampleShader->use();
    mUpsampleShader->setInt("srcTexture", 0);
    mUpsampleShader->setFloat("filterRadius", mFilterRadius);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);
    
    for (int i = (int)mMipChain.size() - 1; i > 0; i--) {
        const BloomMip& mip = mMipChain[i];
        const BloomMip& nextMip = mMipChain[i - 1];
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mip.texture);
        
        glViewport(0, 0, (int)nextMip.size.x, (int)nextMip.size.y);
        glBindFramebuffer(GL_FRAMEBUFFER, nextMip.fbo);
        
        glBindVertexArray(mQuadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
    glDisable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void BloomRenderer::renderToScreen() {
    glViewport(0, 0, mWidth, mHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    mCompositeShader->use();
    mCompositeShader->setInt("sceneTexture", 0);
    mCompositeShader->setInt("bloomTexture", 1);
    mCompositeShader->setFloat("bloomStrength", mBloomStrength);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mHdrColorBuffer);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mMipChain[0].texture);
    
    glBindVertexArray(mQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void BloomRenderer::resize(unsigned int width, unsigned int height) {
    mWidth = width;
    mHeight = height;
    cleanup();
    init();
}