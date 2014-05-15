//
//  Shader.fsh
//  guruchat
//
//  Created by Ivan Borisenko on 07.05.14.
//  Copyright (c) 2014 aeonsoftech. All rights reserved.
//

varying lowp vec4 colorVarying;

void main()
{
    gl_FragColor = colorVarying;
}
