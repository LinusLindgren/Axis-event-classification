function [ cell_histograms ] = calculate_all_cell_histograms( I )
%CALCULATE_CELL_FEATURES Summary of this function goes here
%   Detailed explanation goes here

horSize = size(I,2);
    verSize = size(I,1);
    stepsize = 8;
    nbrHorSteps = floor((horSize)/stepsize);
    nbrVerSteps = floor((verSize)/stepsize);
cell_histograms = zeros(nbrVerSteps,9,nbrHorSteps);
%calculate histogram for every cell

for cj=0:nbrHorSteps-1
    for ci =0:nbrVerSteps-1
        %histo = zeros(1,9);
        %For every pixel in the cell, calculate gradient, put gradient into
        %cell histogram.
        %Edges of cells are ommited due to not having pixels at all sides.
        %This is the case for all cells calculated atm.
        for i=1:8
            for j=1:8
                if(~isBorder(cj,ci,i,j,nbrVerSteps,nbrHorSteps)) 
                %Keep track of which cell we are operating in, therefore the
                %ci*8+ (local cell calculation)
                dyR = I(ci*8+i+1,cj*8+j,1) - I(ci*8+i-1,cj*8+j,1);
                dyG = I(ci*8+i+1,cj*8+j,2) - I(ci*8+i-1,cj*8+j,2);
                dyB = I(ci*8+i+1,cj*8+j,3) - I(ci*8+i-1,cj*8+j,3);
                dxR = I(ci*8+i,cj*8+j+1,1) - I(ci*8+i,cj*8+j-1,1);
                dxG = I(ci*8+i,cj*8+j+1,2) - I(ci*8+i,cj*8+j-1,2);
                dxB = I(ci*8+i,cj*8+j+1,3) - I(ci*8+i,cj*8+j-1,3);
                angs = ones(1,3);
                mags = ones(1,3);
                [angs(1),mags(1)] = cart2pol(dxR,dyR);
                [angs(2),mags(2)] = cart2pol(dxG,dyG);
                [angs(3),mags(3)] = cart2pol(dxB,dyB);
                
                [maxMag,indx] = max(mags);
                %[maxDimDiff,index] = max(hypot(dxR,dxY),hypot(dxG,dyG),hypot(dxB,dyB));
                maxDimAngs = angs(indx);
                
                
                if maxDimAngs < 0
                    unsigned_ang = maxDimAngs+pi;
                    
                else
                    unsigned_ang = maxDimAngs;
                    
                end
                bin_nbr = floor(unsigned_ang / (pi/9));
                
                rest = unsigned_ang/(pi/9) - bin_nbr;
                 if bin_nbr == 9
                    bin_nbr=0; 
                 end
                cell_histograms(ci+1, bin_nbr+1,cj+1) = cell_histograms(ci+1,bin_nbr+1,cj+1) + (1 - rest) * maxMag;
                if bin_nbr == 8
                    bin_nbr = -1;
                end
                cell_histograms(ci+1, bin_nbr+2,cj+1) =cell_histograms(ci+1, bin_nbr+2, cj+1) + rest*maxMag;
       end
            
            
            end 
        end
    end
    
end

end

function [isBorder] = isBorder(cj, ci, i , j,nbrVerSteps,nbrhorSteps)
    isBorder = 0;
    if (cj == 0  & j == 1) | (cj == nbrhorSteps-1 & j == 8) | (ci == 0 & i == 1) | (ci == nbrVerSteps-1 & i == 8) 
            isBorder = 1;
    end
end




