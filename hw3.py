import sys
import os
import matplotlib.pyplot as plt
import numpy as np 
import math 
 
class nn_linear_layer:
    
    # linear layer.
    # randomly initialized by creating matrix W and bias b
    def __init__(self, input_size, output_size, std=1):
        self.W = np.random.normal(0,std,(output_size,input_size))
        self.b = np.random.normal(0,std,(output_size,1))
        
    ######
    ## Q1
    def forward(self,x):
        y = x @ self.W.T + self.b.T
        #print("NN_LINEAR_LAYER FORWARD")
        return y
    
    ######
    ## Q2
    ## returns three parameters
    def backprop(self,x,dLdy):
        dLdx = dLdy @ self.W # 이게 사실은 [ [ W 0 ... 0 ] ... [ 0 ... W ] ] 같은 꼴이지만 dLdy랑 곱해지면 그냥 W를 곱하는 것과 다를 바가 없어 
        dLdW = dLdy.T @ x 
        dLdb = np.sum(dLdy, axis=0, keepdims=True) # 이거 문제다 
        
        #print("NN_LINEAR_LAYER BACKPROP")
        #print(f"dLdW shape: {dLdW.shape} dLdb shape: {dLdb.shape} dLdx shape: {dLdx.shape}")
        return dLdW,dLdb,dLdx

    def update_weights(self,dLdW,dLdb):

        # parameter update
        self.W=self.W+dLdW
        self.b=self.b+dLdb

class nn_activation_layer:
    
    def __init__(self):
        pass
    
    ######
    ## Q3
    def forward(self,x):
        #print("NN_ACTIVATION_LAYER FORWARD")
        #return 1 / (1 + np.exp(-x)) # np.exp는 element-wise 연산 
        return np.where(x >= 0, 
                            1 / (1 + np.exp(-x)), 
                            np.exp(x) / (1 + np.exp(x)))
    
    ######
    ## Q4
    def backprop(self,x,dLdy):
                
        def sigmoid(x):
            # return 1/(1+np.exp(-x))
            return np.where(x >= 0, 
                            1 / (1 + np.exp(-x)), 
                            np.exp(x) / (1 + np.exp(x)))
        
        s = sigmoid(x)
        dLdx = dLdy * (s * (1-s)) # 모두 element-wise로 연산되니까 
        #print("NN_ACTIVATION_LAYER BACKPROP")
        #print(f"dLdx shape: {dLdx.shape}")
        return dLdx 


class nn_softmax_layer:
    def __init__(self):
        pass
    ######
    ## Q5
    def forward(self,x):
        # 각 row 별로 softmax 적용 
        def softmax(x): # for each row 
            exp_x = np.exp(x) # x 전체의 element에 exp 적용             
            sum_exp_x = np.sum(exp_x, axis=1, keepdims=True) 
            return exp_x / sum_exp_x
        #print("NN_SOFTMAX_LAYER FORWARD")
        return softmax(x)
            
    ######
    ## Q6
    def backprop(self,x,dLdy):
        B, _ = x.shape
        def softmax(x): # for each row 
            exp_x = np.exp(x) # x 전체의 element에 exp 적용             
            sum_exp_x = np.sum(exp_x, axis=1, keepdims=True) 
            return exp_x / sum_exp_x
        y = softmax(x)
        dLdx = np.zeros_like(x)

        for i in range(B):
            y_i = y[i].reshape(-1, 1) # (O, 1) vector 꼴로 
            J_i = np.diagflat(y_i) - y_i @ y_i.T 
            dLdx[i] = J_i.T @ dLdy[i] # row를 기준으로 하고 있기 때문에 dLdy dydx가 아니라 그 반대 
        #print("NN_SOFTMAX_LAYER BACKPROP")
        #print(f"dLdx shape: {dLdx.shape}")
        return dLdx 

class nn_cross_entropy_layer:
    def __init__(self):
        pass
        
    ######
    ## Q7
    def forward(self,x,y):
        # y는 (B, 1)의 꼴 
        # x는 (B, I)의 꼴 
        # 각 row에 대해서 cross entropy loss를 적용하고 마지막에 y에 있는 모든 값을 더하면 돼 
        B = x.shape[0]
        log_x = np.log(x + 1e-15)  # log(0) 을 피하기 위하여 
        loss = -log_x[np.arange(B), y.flatten()]  # (B,)
        #print("NN_CROSS_ENTROPY_LAYER FORWARD")
        return np.mean(loss)  # 평균 스칼라 

        
        
    ######
    ## Q8
    def backprop(self,x,y):
        # 애초에 dL/dy로 들어오는게 없어 - upstream이 없어 
        dLdx = np.zeros_like(x) # 우선 x와 동일한 shape를 지니도록 
        B = x.shape[0]
        for i in range(B): # x는 batch로 구성되기 때문에 각 data에 대해서 
            yi = y[i, 0] # ith-data에 대한 answer 
            dLdx[i, yi] = -1/x[i, yi]
        #print("NN_CROSS_ENTROPY_LAYER BACKPROP")
        #print(f"dLdx shape: {dLdx.shape}")
        dLdx /= B 
        return dLdx 

if __name__ == '__main__':

    # number of data points for each of (0,0), (0,1), (1,0) and (1,1)
    num_d=5

    # number of test runs
    num_test=40

    ## Q9. Hyperparameter setting
    ## learning rate (lr)and number of gradient descent steps (num_gd_step)
    ## This part is not graded (there is no definitive answer).
    ## You can set this hyperparameters through experiments.
    lr= 0.01
    num_gd_step= 50000

    # dataset size
    batch_size=4*num_d

    # number of classes is 2
    num_class=2

    # variable to measure accuracy
    accuracy=0

    # set this True if want to plot training data
    show_train_data=False

    # set this True if want to plot loss over gradient descent iteration
    show_loss=True

    ################
    # create training data
    ################

    m_d1 = (0, 0)
    m_d2 = (1, 1)
    m_d3 = (0, 1)
    m_d4 = (1, 0)

    sig = 0.05
    s_d1 = sig ** 2 * np.eye(2)

    d1 = np.random.multivariate_normal(m_d1, s_d1, num_d)
    d2 = np.random.multivariate_normal(m_d2, s_d1, num_d)
    d3 = np.random.multivariate_normal(m_d3, s_d1, num_d)
    d4 = np.random.multivariate_normal(m_d4, s_d1, num_d)

    # training data, and has shape (4*num_d,2)
    x_train_d = np.vstack((d1, d2, d3, d4))
    # training data lables, and has shape (4*num_d,1)
    y_train_d = np.vstack((np.zeros((2 * num_d, 1), dtype='uint8'), np.ones((2 * num_d, 1), dtype='uint8')))

    if (show_train_data):
        plt.grid()
        plt.scatter(x_train_d[range(2 * num_d), 0], x_train_d[range(2 * num_d), 1], color='b', marker='o')
        plt.scatter(x_train_d[range(2 * num_d, 4 * num_d), 0], x_train_d[range(2 * num_d, 4 * num_d), 1], color='r',
                    marker='x')
        plt.show()

    ################
    # create layers
    ################

    # hidden layer
    # linear layer
    layer1 = nn_linear_layer(input_size=2, output_size=4, )
    # activation layer
    act = nn_activation_layer()

    # output layer
    # linear
    layer2 = nn_linear_layer(input_size=4, output_size=2, )
    # softmax
    smax = nn_softmax_layer()
    # cross entropy
    cent = nn_cross_entropy_layer()

    # variable for plotting loss
    loss_out = np.zeros((num_gd_step))

    ################
    # do training
    ################

    for i in range(num_gd_step):
        
        # fetch data
        x_train = x_train_d
        y_train = y_train_d
            
        ################
        # forward pass
        
        # hidden layer
        # linear
        l1_out = layer1.forward(x_train)
        # activation
        a1_out = act.forward(l1_out)
        
        # output layer
        # linear
        l2_out = layer2.forward(a1_out)
        # softmax
        smax_out = smax.forward(l2_out)
        # cross entropy loss
        loss_out[i] = cent.forward(smax_out, y_train)
        
        ################
        # perform backprop
        # output layer
        # cross entropy
        b_cent_out = cent.backprop(smax_out, y_train)
        # softmax
        b_nce_smax_out = smax.backprop(l2_out, b_cent_out)
        
        # linear
        b_dLdW_2, b_dLdb_2, b_dLdx_2 = layer2.backprop(x=a1_out, dLdy=b_nce_smax_out)
        
        # backprop, hidden layer
        # activation
        b_act_out = act.backprop(x=l1_out, dLdy=b_dLdx_2)
        # linear
        b_dLdW_1, b_dLdb_1, b_dLdx_1 = layer1.backprop(x=x_train, dLdy=b_act_out)
        
        ################
        # update weights: perform gradient descent
        layer2.update_weights(dLdW=-b_dLdW_2 * lr, dLdb=-b_dLdb_2.T * lr)
        layer1.update_weights(dLdW=-b_dLdW_1 * lr, dLdb=-b_dLdb_1.T * lr)
        
        if (i + 1) % 2000 == 0:
            print('gradient descent iteration:', i + 1)

    # set show_loss to True to plot the loss over gradient descent iterations
    if (show_loss):
        plt.figure(1)
        plt.grid()
        plt.plot(range(num_gd_step), loss_out)
        plt.xlabel('number of gradient descent steps')
        plt.ylabel('cross entropy loss')
        plt.show()

    ################
    # training done
    # now testing

    num_test = 100

    for j in range(num_test):
        
        predicted = np.ones((4,))
        
        # dispersion of test data
        sig_t = 1e-2
        
        # generate test data
        # generate 4 samples, each sample nearby (1,1), (0,0), (1,0), (0,1) respectively
        t11 = np.random.multivariate_normal((1,1), sig_t**2*np.eye(2), 1)
        t00 = np.random.multivariate_normal((0,0), sig_t**2*np.eye(2), 1)
        t10 = np.random.multivariate_normal((1,0), sig_t**2*np.eye(2), 1)
        t01 = np.random.multivariate_normal((0,1), sig_t**2*np.eye(2), 1)
        
        # predicting label for test sample nearby (1,1)
        l1_out = layer1.forward(t11)
        a1_out = act.forward(l1_out)
        l2_out = layer2.forward(a1_out)
        smax_out = smax.forward(l2_out)
        predicted[0] = np.argmax(smax_out)
        print('softmax out for (1,1)', smax_out, 'predicted label:', int(predicted[0]))
        
        # predicting label for test sample nearby (0,0)
        l1_out = layer1.forward(t00)
        a1_out = act.forward(l1_out)
        l2_out = layer2.forward(a1_out)
        smax_out = smax.forward(l2_out)
        predicted[1] = np.argmax(smax_out)
        print('softmax out for (0,0)', smax_out, 'predicted label:', int(predicted[1]))
        
        # predicting label for test sample nearby (1,0)
        l1_out = layer1.forward(t10)
        a1_out = act.forward(l1_out)
        l2_out = layer2.forward(a1_out)
        smax_out = smax.forward(l2_out)
        predicted[2] = np.argmax(smax_out)
        print('softmax out for (1,0)', smax_out, 'predicted label:', int(predicted[2]))
        
        # predicting label for test sample nearby (0,1)
        l1_out = layer1.forward(t01)
        a1_out = act.forward(l1_out)
        l2_out = layer2.forward(a1_out)
        smax_out = smax.forward(l2_out)
        predicted[3] = np.argmax(smax_out)
        print('softmax out for (0,1)', smax_out, 'predicted label:', int(predicted[3]))
        
        print('total predicted labels:', predicted.astype('uint8'))
        
        accuracy += (predicted[0] == 0) & (predicted[1] == 0) & (predicted[2] == 1) & (predicted[3] == 1)
        
        if (j + 1) % 10 == 0:
            print('test iteration:', j + 1)

    print('accuracy:', accuracy / num_test * 100, '%')






